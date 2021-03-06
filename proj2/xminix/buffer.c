/*
 *  reconstrução de procedimentos de leitura e escrita em disco
 */

#include "xminix.h"
#include "security.h"

static void end_buffer_async_read(struct buffer_head *bh, int uptodate);
static void buffer_io_error(struct buffer_head *bh, char *msg);
static inline int block_size_bits(unsigned int blocksize);
static struct buffer_head *create_page_buffers(struct page *page, struct inode *inode, unsigned int b_state);
static void xminix_do_invalidatepage(struct page *page, unsigned int offset, unsigned int length);
static void xminix_end_bio_bh_io_sync(struct bio *bio);
static void mark_buffer_async_read(struct buffer_head *bh);
static int xminix_submit_bh(int rw, struct buffer_head *bh);
static void xminix_mark_buffer_async_write_endio(struct buffer_head *bh, bh_end_io_t *handler);
static void guard_bio_eod(int rw, struct bio *bio);
static int xminix_submit_bh_wbc(int rw, struct buffer_head *bh, unsigned long bio_flags, struct writeback_control *wbc);
static void xminix_end_buffer_async_write(struct buffer_head *bh, int uptodate);
static int xminix__block_write_full_page(struct inode *inode, struct page *page, get_block_t *get_block, struct writeback_control *wbc, bh_end_io_t *handler);

int xminix_block_read_full_page(struct page *page, get_block_t *get_block)
{
	struct inode *inode = page->mapping->host;
	sector_t iblock, lblock;
	struct buffer_head *bh, *head, *arr[MAX_BUF_PER_PAGE];
	unsigned int blocksize, bbits;
	int nr, i;
	int fully_mapped = 1;

	head = create_page_buffers(page, inode, 0);
	blocksize = head->b_size;
	bbits = block_size_bits(blocksize);

	iblock = (sector_t)page->index << (PAGE_CACHE_SHIFT - bbits);
	lblock = (i_size_read(inode)+blocksize-1) >> bbits;
	bh = head;
	nr = 0;
	i = 0;

	do {		
		if (buffer_uptodate(bh))
			continue;

		if (!buffer_mapped(bh)) {
			int err = 0;

			fully_mapped = 0;
			if (iblock < lblock) {
				WARN_ON(bh->b_size != blocksize);
				err = get_block(inode, iblock, bh, 0);
				if (err)
					SetPageError(page);
			}
			if (!buffer_mapped(bh)) {
				zero_user(page, i * blocksize, blocksize);
				if (!err)
					set_buffer_uptodate(bh);
				continue;
			}			
			
			/*
			 * get_block() might have updated the buffer
			 * synchronously
			 */
			if (buffer_uptodate(bh))
				continue;
		}
		arr[nr++] = bh;
	} while (i++, iblock++, (bh = bh->b_this_page) != head);

	if (fully_mapped)
		SetPageMappedToDisk(page);

	if (!nr) {
		/*
		 * All buffers are uptodate - we can set the page uptodate
		 * as well. But not if get_block() returned an error.
		 */
		if (!PageError(page))
			SetPageUptodate(page);
		unlock_page(page);
		return 0;
	}

	/* Stage two: lock the buffers */
	for (i = 0; i < nr; i++) {
		bh = arr[i];
		lock_buffer(bh);
		mark_buffer_async_read(bh);
	}

	/*
	 * Stage 3: start the IO.  Check for uptodateness
	 * inside the buffer lock in case another process reading
	 * the underlying blockdev brought it uptodate (the sct fix).
	 */
	for (i = 0; i < nr; i++) {
		bh = arr[i];
		
		if (buffer_uptodate(bh))
			end_buffer_async_read(bh, 1);	
		else
			xminix_submit_bh(READ, bh);		
	}
	return 0;
}

int xminix_block_write_full_page(struct page *page, get_block_t *get_block,
			struct writeback_control *wbc)
{
	struct inode * const inode = page->mapping->host;
	loff_t i_size = i_size_read(inode);
	const pgoff_t end_index = i_size >> PAGE_CACHE_SHIFT;
	unsigned offset;

	/* Is the page fully inside i_size? */
	if (page->index < end_index)
		return xminix__block_write_full_page(inode, page, get_block, wbc,
					       xminix_end_buffer_async_write);

	/* Is the page fully outside i_size? (truncate in progress) */
	offset = i_size & (PAGE_CACHE_SIZE-1);
	if (page->index >= end_index+1 || !offset) {
		/*
		 * The page may have dirty, unmapped buffers.  For example,
		 * they may have been added in ext3_writepage().  Make them
		 * freeable here, so the page does not leak.
		 */
		xminix_do_invalidatepage(page, 0, PAGE_CACHE_SIZE);
		unlock_page(page);
		return 0; /* don't care */
	}

	/*
	 * The page straddles i_size.  It must be zeroed out on each and every
	 * writepage invocation because it may be mmapped.  "A file is mapped
	 * in multiples of the page size.  For a file that is not a multiple of
	 * the  page size, the remaining memory is zeroed when mapped, and
	 * writes to that region are not written out to the file."
	 */
	zero_user_segment(page, offset, PAGE_CACHE_SIZE);
	return xminix__block_write_full_page(inode, page, get_block, wbc,
							xminix_end_buffer_async_write);
}

static int xminix__block_write_full_page(struct inode *inode, struct page *page,
			get_block_t *get_block, struct writeback_control *wbc,
			bh_end_io_t *handler)
{
	int err;
	sector_t block;
	sector_t last_block;
	struct buffer_head *bh, *head;
	unsigned int blocksize, bbits;
	int nr_underway = 0;
	int write_op = (wbc->sync_mode == WB_SYNC_ALL ? WRITE_SYNC : WRITE);

	head = create_page_buffers(page, inode,
					(1 << BH_Dirty)|(1 << BH_Uptodate));

	/*
	 * Be very careful.  We have no exclusion from __set_page_dirty_buffers
	 * here, and the (potentially unmapped) buffers may become dirty at
	 * any time.  If a buffer becomes dirty here after we've inspected it
	 * then we just miss that fact, and the page stays dirty.
	 *
	 * Buffers outside i_size may be dirtied by __set_page_dirty_buffers;
	 * handle that here by just cleaning them.
	 */

	bh = head;
	blocksize = bh->b_size;
	bbits = block_size_bits(blocksize);

	block = (sector_t)page->index << (PAGE_CACHE_SHIFT - bbits);
	last_block = (i_size_read(inode) - 1) >> bbits;

	/*
	 * Get all the dirty buffers mapped to disk addresses and
	 * handle any aliases from the underlying blockdev's mapping.
	 */
	do {
		if (block > last_block) {
			/*
			 * mapped buffers outside i_size will occur, because
			 * this page can be outside i_size when there is a
			 * truncate in progress.
			 */
			/*
			 * The buffer was zeroed by block_write_full_page()
			 */
			clear_buffer_dirty(bh);
			set_buffer_uptodate(bh);
		} else if ((!buffer_mapped(bh) || buffer_delay(bh)) &&
			   buffer_dirty(bh)) {
			WARN_ON(bh->b_size != blocksize);
			err = get_block(inode, block, bh, 1);
			if (err)
				goto recover;
			clear_buffer_delay(bh);
			if (buffer_new(bh)) {
				/* blockdev mappings never come here */
				clear_buffer_new(bh);
				unmap_underlying_metadata(bh->b_bdev,
							bh->b_blocknr);
			}
		}
		bh = bh->b_this_page;
		block++;
	} while (bh != head);

	do {
		if (!buffer_mapped(bh))
			continue;
		/*
		 * If it's a fully non-blocking write attempt and we cannot
		 * lock the buffer then redirty the page.  Note that this can
		 * potentially cause a busy-wait loop from writeback threads
		 * and kswapd activity, but those code paths have their own
		 * higher-level throttling.
		 */
		if (wbc->sync_mode != WB_SYNC_NONE) {
			lock_buffer(bh);
		} else if (!trylock_buffer(bh)) {
			redirty_page_for_writepage(wbc, page);
			continue;
		}
		if (test_clear_buffer_dirty(bh)) {
			xminix_mark_buffer_async_write_endio(bh, handler);
		} else {
			unlock_buffer(bh);
		}
	} while ((bh = bh->b_this_page) != head);

	/*
	 * The page and its buffers are protected by PageWriteback(), so we can
	 * drop the bh refcounts early.
	 */
	BUG_ON(PageWriteback(page));
	set_page_writeback(page);

	do {
		struct buffer_head *next = bh->b_this_page;
		if (buffer_async_write(bh)) {
			////////////////////////////////////////////////////////
			//insira criptografia aqui
			printk("[WRITE] Antes criptografia ");			
			dump_buffer(bh->b_data, bh->b_size);
			
			aes_operation(AES_ENCRYPT, bh->b_data, bh->b_size);
	
			printk("[WRITE] Depois criptografia ");
			dump_buffer(bh->b_data, bh->b_size);	
			////////////////////////////////////////////////////////
	
			xminix_submit_bh_wbc(write_op, bh, 0, wbc);
			nr_underway++;
		}
		bh = next;
	} while (bh != head);
	unlock_page(page);

	err = 0;
done:
	if (nr_underway == 0) {
		/*
		 * The page was marked dirty, but the buffers were
		 * clean.  Someone wrote them back by hand with
		 * ll_rw_block/submit_bh.  A rare case.
		 */
		end_page_writeback(page);

		/*
		 * The page and buffer_heads can be released at any time from
		 * here on.
		 */
	}
	return err;

recover:
	/*
	 * ENOSPC, or some other error.  We may already have added some
	 * blocks to the file, so we need to write these out to avoid
	 * exposing stale data.
	 * The page is currently locked and not marked for writeback
	 */
	bh = head;
	/* Recovery: lock and submit the mapped buffers */
	do {
		if (buffer_mapped(bh) && buffer_dirty(bh) &&
		    !buffer_delay(bh)) {
			lock_buffer(bh);
			xminix_mark_buffer_async_write_endio(bh, handler);
		} else {
			/*
			 * The buffer may have been set dirty during
			 * attachment to a dirty page.
			 */
			clear_buffer_dirty(bh);
		}
	} while ((bh = bh->b_this_page) != head);
	SetPageError(page);
	BUG_ON(PageWriteback(page));
	mapping_set_error(page->mapping, err);
	set_page_writeback(page);
	do {
		struct buffer_head *next = bh->b_this_page;
		if (buffer_async_write(bh)) {
			clear_buffer_dirty(bh);
			xminix_submit_bh_wbc(write_op, bh, 0, wbc);
			nr_underway++;
		}
		bh = next;
	} while (bh != head);
	unlock_page(page);
	goto done;
}

static void end_buffer_async_read(struct buffer_head *bh, int uptodate)
{
	printk("end_buffer_async_read\n");
	
	unsigned long flags;
	struct buffer_head *first;
	struct buffer_head *tmp;
	struct page *page;
	int page_uptodate = 1;
	
	BUG_ON(!buffer_async_read(bh));
	
	page = bh->b_page;
	if (uptodate) {
		set_buffer_uptodate(bh);
	} else {
		clear_buffer_uptodate(bh);
		buffer_io_error(bh, ", async page read");
		SetPageError(page);
	}	
		
	/*
	 * Be _very_ careful from here on. Bad things can happen if
	 * two buffer heads end IO at almost the same time and both
	 * decide that the page is now completely done.
	 */
	first = page_buffers(page);
	local_irq_save(flags);
	bit_spin_lock(BH_Uptodate_Lock, &first->b_state);
	
	///////////////////////////////////////////////////////////////////////////
	// insira descriptografia aqui	
	printk("[READ] Antes Descriptografia ");			
	dump_buffer(bh->b_data, bh->b_size);

	aes_operation(AES_DECRYPT, bh->b_data, bh->b_size);
	
	printk("[READ] Depois Descriptografia ");			
	dump_buffer(bh->b_data, bh->b_size);
	///////////////////////////////////////////////////////////////////////////	
	
	clear_buffer_async_read(bh);
	
	unlock_buffer(bh);
			
	tmp = bh;
	do {
		if (!buffer_uptodate(tmp))
			page_uptodate = 0;
		if (buffer_async_read(tmp)) {
			BUG_ON(!buffer_locked(tmp));
			goto still_busy;
		}
				
		tmp = tmp->b_this_page;
	} while (tmp != bh);
		
	bit_spin_unlock(BH_Uptodate_Lock, &first->b_state);
	local_irq_restore(flags);	
		
	/*
	 * If none of the buffers had errors and they are all
	 * uptodate then we can set the page uptodate.
	 */
	if (page_uptodate && !PageError(page))
		SetPageUptodate(page);
	unlock_page(page);
		
	return;

still_busy:
	bit_spin_unlock(BH_Uptodate_Lock, &first->b_state);
	local_irq_restore(flags);
	return;
}

static void buffer_io_error(struct buffer_head *bh, char *msg)
{
	char b[BDEVNAME_SIZE];

	if (!test_bit(BH_Quiet, &bh->b_state))
		//printk_ratelimited(KERN_ERR // removido para simplificar reconstrucao
		printk(KERN_ERR
			"Buffer I/O error on dev %s, logical block %llu%s\n",
			bdevname(bh->b_bdev, b),
			(unsigned long long)bh->b_blocknr, msg);
}

static inline int block_size_bits(unsigned int blocksize)
{
	return ilog2(blocksize);
}

static struct buffer_head *create_page_buffers(struct page *page, struct inode *inode, unsigned int b_state)
{
	BUG_ON(!PageLocked(page));

	if (!page_has_buffers(page))
		create_empty_buffers(page, 1 << ACCESS_ONCE(inode->i_blkbits), b_state);
	return page_buffers(page);
}

static void xminix_do_invalidatepage(struct page *page, unsigned int offset,
		       unsigned int length)
{
	void (*invalidatepage)(struct page *, unsigned int, unsigned int);

	invalidatepage = page->mapping->a_ops->invalidatepage;
#ifdef CONFIG_BLOCK
	if (!invalidatepage)
		invalidatepage = block_invalidatepage;
#endif
	if (invalidatepage)
		(*invalidatepage)(page, offset, length);
}

static void xminix_end_bio_bh_io_sync(struct bio *bio)
{
	printk("xminix_end_bio_bh_io_sync\n");			
	
	struct buffer_head *bh = bio->bi_private;

	if (unlikely(bio_flagged(bio, BIO_QUIET)))
		set_bit(BH_Quiet, &bh->b_state);
		
	bh->b_end_io(bh, !bio->bi_error);
	
	bio_put(bio);
}

static void mark_buffer_async_read(struct buffer_head *bh)
{	
	bh->b_end_io = end_buffer_async_read;
	set_buffer_async_read(bh);
}

static int xminix_submit_bh(int rw, struct buffer_head *bh)
{
	return xminix_submit_bh_wbc(rw, bh, 0, NULL);
}

static void xminix_mark_buffer_async_write_endio(struct buffer_head *bh,
					  bh_end_io_t *handler)
{
	bh->b_end_io = handler;
	set_buffer_async_write(bh);
}

static void guard_bio_eod(int rw, struct bio *bio)
{
	sector_t maxsector;
	struct bio_vec *bvec = &bio->bi_io_vec[bio->bi_vcnt - 1];
	unsigned truncated_bytes;

	maxsector = i_size_read(bio->bi_bdev->bd_inode) >> 9;
	if (!maxsector)
		return;

	/*
	 * If the *whole* IO is past the end of the device,
	 * let it through, and the IO layer will turn it into
	 * an EIO.
	 */
	if (unlikely(bio->bi_iter.bi_sector >= maxsector))
		return;

	maxsector -= bio->bi_iter.bi_sector;
	if (likely((bio->bi_iter.bi_size >> 9) <= maxsector))
		return;

	/* Uhhuh. We've got a bio that straddles the device size! */
	truncated_bytes = bio->bi_iter.bi_size - (maxsector << 9);

	/* Truncate the bio.. */
	bio->bi_iter.bi_size -= truncated_bytes;
	bvec->bv_len -= truncated_bytes;

	/* ..and clear the end of the buffer for reads */
	if ((rw & RW_MASK) == READ) {
		zero_user(bvec->bv_page, bvec->bv_offset + bvec->bv_len,
				truncated_bytes);
	}
}

static int xminix_submit_bh_wbc(int rw, struct buffer_head *bh,
			 unsigned long bio_flags, struct writeback_control *wbc)
{
	struct bio *bio;

	BUG_ON(!buffer_locked(bh));
	BUG_ON(!buffer_mapped(bh));
	BUG_ON(!bh->b_end_io);
	BUG_ON(buffer_delay(bh));
	BUG_ON(buffer_unwritten(bh));
	
	/*
	 * Only clear out a write error when rewriting
	 */
	if (test_set_buffer_req(bh) && (rw & WRITE))
		clear_buffer_write_io_error(bh);

	/*
	 * from here on down, it's all bio -- do the initial mapping,
	 * submit_bio -> generic_make_request may further map this bio around
	 */
	bio = bio_alloc(GFP_NOIO, 1);

	if (wbc) {
		wbc_init_bio(wbc, bio);
		wbc_account_io(wbc, bh->b_page, bh->b_size);
	}

	bio->bi_iter.bi_sector = bh->b_blocknr * (bh->b_size >> 9);
	bio->bi_bdev = bh->b_bdev;

	bio_add_page(bio, bh->b_page, bh->b_size, bh_offset(bh));
	BUG_ON(bio->bi_iter.bi_size != bh->b_size);

	bio->bi_end_io = xminix_end_bio_bh_io_sync;
	bio->bi_private = bh;
	bio->bi_flags |= bio_flags;

	/* Take care of bh's that straddle the end of the device */
	guard_bio_eod(rw, bio);

	if (buffer_meta(bh))
		rw |= REQ_META;
	if (buffer_prio(bh))
		rw |= REQ_PRIO;

	submit_bio(rw, bio);
	return 0;
}

static void xminix_end_buffer_async_write(struct buffer_head *bh, int uptodate)
{
	unsigned long flags;
	struct buffer_head *first;
	struct buffer_head *tmp;
	struct page *page;

	BUG_ON(!buffer_async_write(bh));

	page = bh->b_page;
	if (uptodate) {
		set_buffer_uptodate(bh);
	} else {
		buffer_io_error(bh, ", lost async page write");
		set_bit(AS_EIO, &page->mapping->flags);
		set_buffer_write_io_error(bh);
		clear_buffer_uptodate(bh);
		SetPageError(page);
	}

	first = page_buffers(page);
	local_irq_save(flags);
	bit_spin_lock(BH_Uptodate_Lock, &first->b_state);

	clear_buffer_async_write(bh);
	
	
	
	unlock_buffer(bh);
	tmp = bh->b_this_page;
	while (tmp != bh) {
		if (buffer_async_write(tmp)) {
			BUG_ON(!buffer_locked(tmp));
			goto still_busy;
		}
		tmp = tmp->b_this_page;
	}
	bit_spin_unlock(BH_Uptodate_Lock, &first->b_state);
	local_irq_restore(flags);
	end_page_writeback(page);
	return;

still_busy:
	bit_spin_unlock(BH_Uptodate_Lock, &first->b_state);
	local_irq_restore(flags);
	return;
}

void dump_buffer(unsigned char *buf, size_t size)
{
	if (!buf) {
		printk("dump_buffer null pointer\n");
		return;
	}
	
	printk("size: %d %x\n", size, buf);
	
	size_t i; 
	for (i = 0; i < size; i++) {
		char c = (char)buf[i];		
		if (c != '\0' && c != '\n' && (c == 9 || c == 10 || (c >= 32 && c <= 127))) {
			printk("%c", (char)c);
		}
		else {
			//printk("\\x%02x", (unsigned char)c);
		}	
	}	
	printk("\n");
}

