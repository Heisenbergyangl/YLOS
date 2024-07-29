$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map \
	$(SRC)/master.sfdisk \
	$(BUILTIN_APPS) \

# 创建一个16M 的硬盘镜像
	yes | ./bin/bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@

# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc

# 将 loader.bin 写入硬盘
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc

# 测试 system.bin 小于 100K，否则需要修改下面的 count
	test -n "$$(find $(BUILD)/system.bin -size -100k)"

# 测试system.bin 写入硬盘
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

# 对硬盘进行分区
	sfdisk $@ < $(SRC)/master.sfdisk

# 挂载设备
	sudo losetup /dev/loop10 --partscan $@

# 创建 minux 文件系统
	sudo mkfs.minix -1 -n 14 /dev/loop10p1

# 挂载文件系统
	sudo mount /dev/loop10p1 /mnt

# 切换所有者
	sudo chown ${USER} /mnt 

# 创建目录
	mkdir -p /mnt/bin
	mkdir -p /mnt/dev
	mkdir -p /mnt/mnt

# 拷贝程序
	for app in $(BUILTIN_APPS); \
	do \
		cp $$app /mnt/bin; \
	done

	echo "hello ylos!!!" > /mnt/hello.txt

# 卸载文件系统
	sudo umount /mnt
# sudo umount /dev/loop10p1

# 卸载设备
	sudo losetup -d /dev/loop10

$(BUILD)/slave.img: $(SRC)/slave.sfdisk 

# 创建一个 32M 的硬盘镜像
	yes | ./bin/bximage -q -hd=32 -func=create -sectsize=512 -imgmode=flat $@

# 执行硬盘分区
	sfdisk $@ < $(SRC)/slave.sfdisk

# 挂载设备
	sudo losetup /dev/loop10 --partscan $@

# 创建 minux 文件系统
	sudo mkfs.minix -1 -n 14 /dev/loop10p1

# 挂载文件系统
	sudo mount /dev/loop10p1 /mnt

# 切换所有者
	sudo chown ${USER} /mnt 

# 创建文件
	echo "slave root direcotry file..." > /mnt/hello.txt

# 卸载文件系统
	sudo umount /mnt
#sudo umount /dev/loop10p1
# 卸载设备
	sudo losetup -d /dev/loop10


.PHONY: mount10
mount10: $(BUILD)/master.img
	sudo losetup /dev/loop10 --partscan $<
	sudo mount /dev/loop10p1 /mnt
	sudo chown ${USER} /mnt 

.PHONY: umount10
umount10: /dev/loop10
	-sudo umount /mnt
	-sudo losetup -d $<

.PHONY: mount11
mount11: $(BUILD)/slave.img
	sudo losetup /dev/loop11 --partscan $<
	sudo mount /dev/loop11p1 /mnt
	sudo chown ${USER} /mnt 

.PHONY: umount11
umount11: /dev/loop11
	-sudo umount /mnt
	-sudo losetup -d $<

IMAGES:= $(BUILD)/master.img \
	$(BUILD)/slave.img \

image: $(IMAGES)


