# 
# Copyright 2022 Capgemini Engineering
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.7.2)
project(liblwip C)

# LWIP files
set(LWIP src)
list(APPEND lwip_core ${LWIP}/core/init.c)
list(APPEND lwip_core ${LWIP}/core/def.c)
list(APPEND lwip_core ${LWIP}/core/dns.c)
list(APPEND lwip_core ${LWIP}/core/inet_chksum.c)
list(APPEND lwip_core ${LWIP}/core/ip.c)
list(APPEND lwip_core ${LWIP}/core/mem.c)
list(APPEND lwip_core ${LWIP}/core/memp.c)
list(APPEND lwip_core ${LWIP}/core/netif.c)
list(APPEND lwip_core ${LWIP}/core/pbuf.c)
list(APPEND lwip_core ${LWIP}/core/raw.c)
list(APPEND lwip_core ${LWIP}/core/stats.c)
list(APPEND lwip_core ${LWIP}/core/sys.c)
list(APPEND lwip_core ${LWIP}/core/altcp.c)
list(APPEND lwip_core ${LWIP}/core/altcp_alloc.c)
list(APPEND lwip_core ${LWIP}/core/altcp_tcp.c)
list(APPEND lwip_core ${LWIP}/core/tcp.c)
list(APPEND lwip_core ${LWIP}/core/tcp_in.c)
list(APPEND lwip_core ${LWIP}/core/tcp_out.c)
list(APPEND lwip_core ${LWIP}/core/timeouts.c)
list(APPEND lwip_core ${LWIP}/core/udp.c)

list(APPEND lwip_core4 ${LWIP}/core/ipv4/autoip.c)
list(APPEND lwip_core4 ${LWIP}/core/ipv4/dhcp.c)
list(APPEND lwip_core4 ${LWIP}/core/ipv4/etharp.c)
list(APPEND lwip_core4 ${LWIP}/core/ipv4/icmp.c)
list(APPEND lwip_core4 ${LWIP}/core/ipv4/igmp.c)
list(APPEND lwip_core4 ${LWIP}/core/ipv4/ip4_frag.c)
list(APPEND lwip_core4 ${LWIP}/core/ipv4/ip4.c)
list(APPEND lwip_core4 ${LWIP}/core/ipv4/ip4_addr.c)

# NETIFFILES: Files implementing various generic network interface functions
list(APPEND netiffiles ${LWIP}/netif/ethernet.c)

# Generate the final list of source files
list(APPEND deps ${lwip_core})
list(APPEND deps ${lwip_core4})
list(APPEND deps ${netiffiles})
list(SORT deps)
add_library(lwip STATIC ${deps})

# Include files
target_include_directories(lwip PUBLIC
    ${LWIP}/include
    ${LWIP}/include/ipv4
)
