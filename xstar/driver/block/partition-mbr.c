/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <kernel/core/logger.h>
#include <driver/block/partition.h>

struct mbr_entry_t {
	uint8_t flag;
	uint8_t start_head;
	uint8_t start_sector;
	uint8_t start_cylinder;
	uint8_t type;
	uint8_t end_head;
	uint8_t end_sector;
	uint8_t end_cylinder;
	uint8_t start[4];
	uint8_t length[4];
} __attribute__ ((packed));

struct mbr_header_t {
	uint8_t code[446];
	struct mbr_entry_t entry[4];
	uint8_t signature[2];
} __attribute__ ((packed));

static int is_extended(uint8_t type)
{
	if((type == 0x5) || (type == 0xf) || (type == 0x85))
		return 1;
	return 0;
}

int partition_detect_mbr(struct block_t * pblk)
{
	struct mbr_header_t mbr;
	struct device_t * dev;
	struct block_t * blk;
	uint64_t offset, length;
	char nbuf[64];
	char sbuf[64];
	int i;

	if(!pblk || !pblk->name || (block_capacity(pblk) <= 0))
		return 0;

	if(block_read(pblk, (uint8_t *)(&mbr), 0, sizeof(struct mbr_header_t)) != sizeof(struct mbr_header_t))
		return 0;

	if((mbr.signature[0] != 0x55) || mbr.signature[1] != 0xaa)
		return 0;

	if((mbr.entry[0].type == 0xee) || (mbr.entry[1].type == 0xee) || (mbr.entry[2].type == 0xee) || (mbr.entry[3].type == 0xee))
		return 0;

	LOG("Found mbr partition:\r\n");
	LOG("  0x%016Lx ~ 0x%016Lx %s %*s- %s\r\n", 0ULL, block_capacity(pblk) - 1, xos_ssize(sbuf, block_capacity(pblk)), 9 - xos_strlen(sbuf), "", pblk->name);
	for(i = 0; i < 4; i++)
	{
		if((mbr.entry[i].type != 0) && (!is_extended(mbr.entry[i].type)))
		{
			xos_snprintf(nbuf, sizeof(nbuf), "p%d", i);
			offset = 512 * ((mbr.entry[i].start[3] << 24) | (mbr.entry[i].start[2] << 16) | (mbr.entry[i].start[1] << 8) | (mbr.entry[i].start[0] << 0));
			length = 512 * ((mbr.entry[i].length[3] << 24) | (mbr.entry[i].length[2] << 16) | (mbr.entry[i].length[1] << 8) | (mbr.entry[i].length[0] << 0));
			dev = register_sub_block(pblk, offset, length, nbuf);
			if(dev)
			{
				blk = (struct block_t *)dev->priv;
				LOG("  0x%016Lx ~ 0x%016Lx %s %*s- %s\r\n", offset, offset + length - 1, xos_ssize(sbuf, length), 9 - xos_strlen(sbuf), "", blk->name);
			}
		}
	}
	return 1;
}
