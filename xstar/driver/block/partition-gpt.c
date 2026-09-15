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

struct gpt_entry_t {
	uint8_t type_uuid[16];
	uint8_t unique_uuid[16];
	uint8_t first_lba[8];
	uint8_t last_lba[8];
	uint8_t attr[8];
	uint8_t name[72];
} __attribute__ ((packed));

struct gpt_header_t {
	uint8_t signature[8];
	uint8_t revision[4];
	uint8_t size[4];
	uint8_t header_crc[4];
	uint8_t reserved[4];
	uint8_t current_lba[8];
	uint8_t backup_lba[8];
	uint8_t first_lba[8];
	uint8_t last_lba[8];
	uint8_t disk_uuid[16];
	uint8_t part_lba[8];
	uint8_t list_num[4];
	uint8_t part_size[4];
	uint8_t part_crc[4];
} __attribute__ ((packed));

static int is_protective_mbr(struct mbr_header_t * mbr)
{
	if((mbr->signature[0] != 0x55) || (mbr->signature[1] != 0xaa))
		return 0;
	if((mbr->entry[0].type != 0xee) && (mbr->entry[1].type != 0xee) && (mbr->entry[2].type != 0xee) && (mbr->entry[3].type != 0xee))
		return 0;
	return 1;
}

static int is_zero_guid(const uint8_t * guid)
{
	for(int i = 0; i < 16; i++)
	{
		if(guid[i] != 0)
			return 0;
	}
	return 1;
}

static void gpt_entry_name(struct gpt_entry_t * entry, char * buf, int size)
{
	int j = 0;

	for(int i = 0; (i < 72) && (j < size - 1); i += 2)
	{
		uint16_t c = get_unaligned_le16(&entry->name[i]);
		if(c == 0)
			break;
		if((c >= 0x20) && (c < 0x7f))
			buf[j++] = (char)c;
	}
	buf[j] = '\0';
}

static void register_gpt_partition(struct block_t * pblk, uint64_t offset, uint64_t length, const char * name, const char * label, char * sbuf)
{
	if((offset < block_capacity(pblk)) && (length > 0) && (length <= block_capacity(pblk) - offset))
	{
		struct device_t * dev = register_sub_block(pblk, offset, length, name);
		if(dev)
		{
			struct block_t * blk = (struct block_t *)dev->priv;
			if(label[0] != '\0')
				LOG("  0x%016Lx ~ 0x%016Lx %s %*s- %s [%s]\r\n", offset, offset + length - 1, xos_ssize(sbuf, length), 9 - xos_strlen(sbuf), "", blk->name, label);
			else
				LOG("  0x%016Lx ~ 0x%016Lx %s %*s- %s\r\n", offset, offset + length - 1, xos_ssize(sbuf, length), 9 - xos_strlen(sbuf), "", blk->name);
		}
	}
}

int partition_detect_gpt(struct block_t * pblk)
{
	struct mbr_header_t mbr;
	uint8_t hbuf[512];
	struct gpt_header_t * gpt = (struct gpt_header_t *)hbuf;
	char sbuf[64];
	char nbuf[40];

	if(!pblk || !pblk->name || (block_capacity(pblk) <= 0))
		return 0;
	uint64_t cap = block_capacity(pblk);

	if(block_read(pblk, (uint8_t *)(&mbr), 0, sizeof(struct mbr_header_t)) != sizeof(struct mbr_header_t))
		return 0;

	if(!is_protective_mbr(&mbr))
		return 0;

	if(block_read(pblk, hbuf, 512, sizeof(hbuf)) != sizeof(hbuf))
		return 0;

	if(xos_memcmp(gpt->signature, "EFI PART", 8) != 0)
		return 0;

	uint32_t header_size = get_unaligned_le32(gpt->size);
	uint32_t header_crc = get_unaligned_le32(gpt->header_crc);
	if((header_size < 92) || (header_size > sizeof(hbuf)))
		return 0;
	put_unaligned_le32(gpt->header_crc, 0);
	if(crc32_sum(0, hbuf, header_size) != header_crc)
		return 0;

	uint64_t part_lba = get_unaligned_le64(gpt->part_lba);
	uint32_t list_num = get_unaligned_le32(gpt->list_num);
	uint32_t part_size = get_unaligned_le32(gpt->part_size);
	uint32_t part_crc = get_unaligned_le32(gpt->part_crc);

	if((part_lba < 2) || (part_size < 128) || (part_size > 512) || ((part_size & 0x7) != 0) || (list_num == 0))
		return 0;

	int crc_check = (list_num <= 128);
	if(list_num > 128)
		list_num = 128;

	uint64_t array_len = (uint64_t)list_num * part_size;
	if((part_lba * 512 >= cap) || ((part_lba * 512 + array_len) > cap))
		return 0;

	uint8_t * buf = xos_mem_malloc(array_len);
	if(!buf)
		return 0;

	if(block_read(pblk, buf, part_lba * 512, array_len) != array_len)
	{
		xos_mem_free(buf);
		return 0;
	}

	if(crc_check && (crc32_sum(0, buf, array_len) != part_crc))
	{
		xos_mem_free(buf);
		return 0;
	}

	LOG("Found gpt partition:\r\n");
	LOG("  0x%016Lx ~ 0x%016Lx %s %*s- %s\r\n", 0ULL, cap - 1, xos_ssize(sbuf, cap), 9 - xos_strlen(sbuf), "", pblk->name);
	for(int i = 0; i < (int)list_num; i++)
	{
		struct gpt_entry_t * entry = (struct gpt_entry_t *)(buf + i * part_size);
		if(is_zero_guid(entry->type_uuid))
			continue;
		uint64_t first = get_unaligned_le64(entry->first_lba);
		uint64_t last = get_unaligned_le64(entry->last_lba);
		if((first > last) || (last >= cap / 512))
			continue;
		xos_snprintf(nbuf, sizeof(nbuf), "p%d", i + 1);
		char lbuf[40];
		gpt_entry_name(entry, lbuf, sizeof(lbuf));
		register_gpt_partition(pblk, first * 512, (last - first + 1) * 512, nbuf, lbuf, sbuf);
	}
	xos_mem_free(buf);
	return 1;
}
