/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <sys/cdefs.h>
#include <sys/socket.h>
__BEGIN_DECLS
struct NetdClientDispatch {
    int (*accept4)(int, struct sockaddr*, socklen_t*, int);
    int (*connect)(int, const struct sockaddr*, socklen_t);
    int (*sendmmsg)(int, const struct mmsghdr*, unsigned int, int);
    ssize_t (*sendmsg)(int, const struct msghdr*, int);
    ssize_t (*sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t);
    int (*socket)(int, int, int);
    unsigned (*netIdForResolv)(unsigned);
    int (*dnsOpenProxy)();
};

static unsigned fallBackNetIdForResolv(unsigned netId) {
    return netId;
}
static int fallBackDnsOpenProxy() {
    return -1;
}

static struct NetdClientDispatch __netdClientDispatch = {
    &accept4,
    &connect,
    &sendmmsg,
    &sendmsg,
    &sendto,
    &socket,
    &fallBackNetIdForResolv,
    &fallBackDnsOpenProxy,
};

__END_DECLS