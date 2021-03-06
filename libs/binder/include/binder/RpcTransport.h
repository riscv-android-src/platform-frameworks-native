/*
 * Copyright (C) 2021 The Android Open Source Project
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

// Wraps the transport layer of RPC. Implementation may use plain sockets or TLS.

#pragma once

#include <memory>
#include <string>

#include <android-base/result.h>
#include <android-base/unique_fd.h>
#include <utils/Errors.h>

namespace android {

class FdTrigger;

// Represents a socket connection.
class RpcTransport {
public:
    virtual ~RpcTransport() = default;

    // replacement of ::recv(MSG_PEEK). Error code may not be set if TLS is enabled.
    virtual android::base::Result<size_t> peek(void *buf, size_t size) = 0;

    /**
     * Read (or write), but allow to be interrupted by a trigger.
     *
     * Return:
     *   OK - succeeded in completely processing 'size'
     *   error - interrupted (failure or trigger)
     */
    virtual status_t interruptableWriteFully(FdTrigger *fdTrigger, const void *buf,
                                             size_t size) = 0;
    virtual status_t interruptableReadFully(FdTrigger *fdTrigger, void *buf, size_t size) = 0;

protected:
    RpcTransport() = default;
};

// Represents the context that generates the socket connection.
class RpcTransportCtx {
public:
    virtual ~RpcTransportCtx() = default;

    // Create a new RpcTransport object.
    //
    // Implemenion details: for TLS, this function may incur I/O. |fdTrigger| may be used
    // to interrupt I/O. This function blocks until handshake is finished.
    [[nodiscard]] virtual std::unique_ptr<RpcTransport> newTransport(
            android::base::unique_fd fd, FdTrigger *fdTrigger) const = 0;

protected:
    RpcTransportCtx() = default;
};

// A factory class that generates RpcTransportCtx.
class RpcTransportCtxFactory {
public:
    virtual ~RpcTransportCtxFactory() = default;
    // Creates server context.
    [[nodiscard]] virtual std::unique_ptr<RpcTransportCtx> newServerCtx() const = 0;

    // Creates client context.
    [[nodiscard]] virtual std::unique_ptr<RpcTransportCtx> newClientCtx() const = 0;

    // Return a short description of this transport (e.g. "raw"). For logging / debugging / testing
    // only.
    [[nodiscard]] virtual const char *toCString() const = 0;

protected:
    RpcTransportCtxFactory() = default;
};

} // namespace android
