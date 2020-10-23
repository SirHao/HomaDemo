/* Copyright (c) 2019, Stanford University
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* This file contains functions that implement the Homa API visible to
 * applications. It's intended to be part of the user-level run-time library.
 */

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "homa.h"

/**
 * homa_recv() - Wait for an incoming message (either request or response)
 * and return it.
 * @sockfd:     File descriptor for the socket on which to receive the message.
 * @buf:        First byte of buffer for the incoming message.
 * @len:        Number of bytes available at @request.
 * @flags:      An ORed combination of bits such as HOMA_RECV_REQUEST and
 *              HOMA_RECV_NONBLOCKING
 * @src_addr:   The sender's IP address will be returned here. If NULL, no
 *              address information is returned.
 * @addrlen:    Space available at @src_addr, in bytes.
 * @id:         Points to a unique RPC identifier, which is used both as
 *              an input and an output parameter. If the value is initially
 *              nonzero and the HOMA_RECV_RESPONSE flag is set, then a
 *              response will not be returned unless it matches this id.
 *              This word is also used to return the id for the incoming
 *              message, whether request or response.
 *
 * Return:      The total size of the incoming message. This may be larger
 *              than len, in which case the last bytes of the incoming message
 *              were discarded. If an error occurred, -1 is returned and
 *              errno is set appropriately.
 */
ssize_t homa_recv(int sockfd, void *buf, size_t len, int flags,
                  struct sockaddr *src_addr, size_t addrlen, uint64_t *id)
{
    struct homa_args_recv_ipv4 args;
    int result;

    if (src_addr && (addrlen < sizeof(struct sockaddr_in))) {
        errno = EINVAL;
        return -EINVAL;
    }
    args.buf = (void *) buf;
    args.len = len;
    args.flags = flags;
    args.id = *id;
    result = ioctl(sockfd, HOMAIOCRECV, &args);
    if (src_addr)
        *((struct sockaddr_in *) src_addr) = args.source_addr;
    *id = args.id;
    return result;
}

/**
 * homa_reply() - Send a response message for an RPC previously received
 * with a call to homa_recv.
 * @sockfd:     File descriptor for the socket on which to send the message.
 * @response:   First byte of buffer containing the response message.
 * @resplen:    Number of bytes at @response.
 * @dest_addr:  Address of the RPC's client (returned by homa_recv when
 *              the message was received).
 * @addrlen:    Size of @dest_addr in bytes.
 * @id:         Unique identifier for the request, as returned by homa_recv
 *              when the request was received.
 *
 * @dest_addr and @id must correspond to a previously-received request
 * for which no reply has yet been sent; if there is no such active request,
 * then this function does nothing.
 *
 * Return:      0 means the response has been accepted for delivery. If an
 *              error occurred, -1 is returned and errno is set appropriately.
 */
ssize_t homa_reply(int sockfd, const void *response, size_t resplen,
                   const struct sockaddr *dest_addr, size_t addrlen,
                   uint64_t id)
{
    struct homa_args_reply_ipv4 args;

    if (dest_addr->sa_family != AF_INET) {
        errno = EAFNOSUPPORT;
        return -EAFNOSUPPORT;
    }
    args.response = (void *) response;
    args.resplen = resplen;
    args.dest_addr = *((struct sockaddr_in *) dest_addr);
    args.id = id;
    return ioctl(sockfd, HOMAIOCREPLY, &args);
}

/**
 * homa_send() - Send a request message to initiate an RPC.
 * @sockfd:     File descriptor for the socket on which to send the message.
 * @request:    First byte of buffer containing the request message.
 * @reqlen:     Number of bytes at @request.
 * @dest_addr:  Address of server to which the request should be sent.
 * @addrlen:    Size of @dest_addr in bytes.
 * @id:         A unique identifier for the request will be returned here;
 *              this can be used later to find the response for this request.
 *
 * Return:      0 means the request has been accepted for delivery. If an
 *              error occurred, -1 is returned and errno is set appropriately.
 */
int homa_send(int sockfd, const void *request, size_t reqlen,
              const struct sockaddr *dest_addr, size_t addrlen,
              uint64_t *id)
{
    struct homa_args_send_ipv4 args;
    int result;

    if (dest_addr->sa_family != AF_INET) {
        errno = EAFNOSUPPORT;
        return -EAFNOSUPPORT;
    }
    args.request = (void *) request;
    args.reqlen = reqlen;
    args.dest_addr = *((struct sockaddr_in *) dest_addr);
    args.id = 0;
    result = ioctl(sockfd, HOMAIOCSEND, &args);
    if (result >= 0)
        *id = args.id;
    return result;
}