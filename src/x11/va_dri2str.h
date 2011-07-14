/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef _DRI2_PROTO_H_
#define _DRI2_PROTO_H_

#define DRI2_NAME			"DRI2"
#define DRI2_MAJOR			1
#define DRI2_MINOR			0

#define DRI2NumberErrors		0
#define DRI2NumberEvents		0
#define DRI2NumberRequests		7

#define X_DRI2QueryVersion		0
#define X_DRI2Connect			1
#define X_DRI2Authenticate		2
#define X_DRI2CreateDrawable		3
#define X_DRI2DestroyDrawable		4
#define X_DRI2GetBuffers		5
#define X_DRI2CopyRegion		6

typedef struct {
    CARD32  attachment B32;
    CARD32  name B32;
    CARD32  pitch B32;
    CARD32  cpp B32;
    CARD32  flags B32;
} xDRI2Buffer;

typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  majorVersion B32;
    CARD32  minorVersion B32;
} xDRI2QueryVersionReq;
#define sz_xDRI2QueryVersionReq   12

typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  majorVersion B32;
    CARD32  minorVersion B32;
    CARD32  pad2 B32;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
} xDRI2QueryVersionReply;
#define sz_xDRI2QueryVersionReply	32

typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  window B32;
    CARD32  drivertype B32;
} xDRI2ConnectReq;
#define sz_xDRI2ConnectReq	12

typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  driverNameLength B32;
    CARD32  deviceNameLength B32;
    CARD32  pad2 B32;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
} xDRI2ConnectReply;
#define sz_xDRI2ConnectReply	32

typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  window B32;
    CARD32  magic B32;
} xDRI2AuthenticateReq;
#define sz_xDRI2AuthenticateReq   12

typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  authenticated B32;
    CARD32  pad2 B32;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
} xDRI2AuthenticateReply;
#define sz_xDRI2AuthenticateReply	32

typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  drawable B32;
} xDRI2CreateDrawableReq;
#define sz_xDRI2CreateDrawableReq   8

typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  drawable B32;
} xDRI2DestroyDrawableReq;
#define sz_xDRI2DestroyDrawableReq   8

typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  drawable B32;
    CARD32  count B32;
} xDRI2GetBuffersReq;
#define sz_xDRI2GetBuffersReq   12

typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  width B32;
    CARD32  height B32;
    CARD32  count B32;
    CARD32  pad2 B32;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
} xDRI2GetBuffersReply;
#define sz_xDRI2GetBuffersReply	32

typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  drawable B32;
    CARD32  region B32;
    CARD32  dest B32;
    CARD32  src B32;
} xDRI2CopyRegionReq;
#define sz_xDRI2CopyRegionReq   20

typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  pad2 B32;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
    CARD32  pad7 B32;
} xDRI2CopyRegionReply;
#define sz_xDRI2CopyRegionReply	32

#endif
