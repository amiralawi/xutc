CFLAGS += -DREG=register -pthread
# -DHZ=${HZ}
# -DPRATTLE


c_src_files := src/xutThread.c src/xutThreadPool.c

xutc: $(c_src_files)
	cc $(c_src_files) $(CFLAGS) -c -O3


clean:
	rm -f *.o
