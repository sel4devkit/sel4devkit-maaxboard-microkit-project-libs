#define dev_dbg(dev, fmt, ...) log_err(fmt, ##__VA_ARGS__)
#define dev_dbg(dev, fmt, ...) \n    log_err(fmt, ##__VA_ARGS__)
#!/bin/bash

SEARCH_DIR="/home/dstorer/test-dev-kit/microkit/example/maaxboard/uboot-driver-example/libubootdrivers"
MACRO="#define dev_dbg(dev, fmt, ...) \\\n    log_err(fmt, ##__VA_ARGS__)"

grep -Rl "dev_dbg" "/home/dstorer/test-dev-kit/microkit/example/maaxboard/uboot-driver-example/libubootdrivers" | while read -r file; do
    sed -i "1i#define dev_dbg(dev, fmt, ...) log_err(fmt, ##__VA_ARGS__)" "$file"
done

grep -Rl "#define dev_dbg(dev, fmt, \.\.\.)" uboot/ | while read -r file; do
    sed -i '/#define dev_dbg(dev, fmt, \.\.\.)/d' "$file"
done


