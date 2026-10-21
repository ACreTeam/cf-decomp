/*
GameSpy GT2 SDK
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 2002 GameSpy Industries, Inc

devsupport@gamespy.com
*/

#include "gt2Message.h"
#include "gt2Buffer.h"
#include "gt2Callback.h"
#include "gt2Connection.h"
#include "gt2Socket.h"
#include "gt2Utility.h"
#include <string.h>

static unsigned short gti2UShortFromBuffer(const GT2Byte *buffer, int pos) {
    unsigned short s;

    s = (unsigned short)((buffer[pos] << 8) | buffer[pos + 1]);

    return s;
}

static void gti2UShortToBuffer(GT2Byte *buffer, int pos, unsigned short s) {
    buffer[pos++] = (GT2Byte)((s >> 8) & 0xFF);
    buffer[pos++] = (GT2Byte)(s & 0xFF);
}

// returns SN1 - SN2, as a signed value
static int gti2SNDiff(unsigned short SN1, unsigned short SN2) {
    return (short)(SN1 - SN2);
}

static GT2Bool gti2ConnectionError(GTI2Connection *connection, GT2Result result,
                                   GT2CloseReason reason) {
    // check if we're still negotiating
    if (connection->state < GTI2Connected) {
        // client-side: tell the app the connection attempt failed
        if (connection->initiated) {
            gti2ConnectionClosed(connection);
            if (!gti2ConnectedCallback(connection, result, NULL, 0)) {
                return GT2False;
            }
        } else {
            // server-side
            if (connection->state == GTI2AwaitingAcceptReject) {
                connection->freeAtAcceptReject = GT2True;
            }

            gti2ConnectionClosed(connection);
        }
    } else if (connection->state != GTI2Closed) {
        gti2ConnectionClosed(connection);
        if (!gti2ClosedCallback(connection, reason)) {
            return GT2False;
        }
    }

    return GT2True;
}

static GT2Bool gti2ConnectionCommunicationError(GTI2Connection *connection) {
    return gti2ConnectionError(connection, GT2NegotiationError,
                               GT2CommunicationError);
}

static GT2Bool gti2ConnectionMemoryError(GTI2Connection *connection) {
    // let the other side know
    if (!gti2SendClosed(connection)) {
        return GT2False;
    }

    return gti2ConnectionError(connection, GT2OutOfMemory, GT2NotEnoughMemory);
}

static GT2Bool gti2HandleESN(GTI2Connection *connection, GT2MessageID ESN) {
    int len;
    int i;
    GTI2OutgoingBufferMessage *message;
    int shortenBy;

    // check if there are any messages in the buffer
    len = ArrayLength(connection->outgoingBufferMessages);
    if (!len) {
        return GT2True;
    }

    // find the first message that hasn't been confirmed
    for (i = 0; i < len; i++) {
        message = (GTI2OutgoingBufferMessage *)ArrayNth(
            connection->outgoingBufferMessages, i);
        if (gti2SNDiff(message->serialNumber, ESN) >= 0) {
            break;
        }
    }

    // check if nothing was confirmed
    if (i == 0) {
        return GT2True;
    }

    // remove the confirmed messages
    while (i--) {
        ArrayDeleteAt(connection->outgoingBufferMessages, i);
    }

    // check if all of the messages were confirmed
    len = ArrayLength(connection->outgoingBufferMessages);
    if (!len) {
        connection->outgoingBuffer.len = 0;
        return GT2True;
    }

    // shift the remaining messages to the start of the buffer
    message = (GTI2OutgoingBufferMessage *)ArrayNth(
        connection->outgoingBufferMessages, 0);
    shortenBy = message->start;
    for (i = 0; i < len; i++) {
        message = (GTI2OutgoingBufferMessage *)ArrayNth(
            connection->outgoingBufferMessages, i);
        message->start -= shortenBy;
    }

    gti2BufferShorten(&connection->outgoingBuffer, 0, shortenBy);

    return GT2True;
}

static GT2Bool gti2HandleAppUnreliable(GTI2Connection *connection,
                                       GT2Byte *message, int len) {
    // check the state
    if ((connection->state != GTI2Connected) &&
        (connection->state != GTI2Closing)) {
        return GT2True;
    }

    // check for filters
    if (ArrayLength(connection->receiveFilters)) {
        if (!gti2ReceiveFilterCallback(connection, 0, message, len, GT2False)) {
            return GT2False;
        }
        return GT2True;
    }

    if (!gti2ReceivedCallback(connection, message, len, GT2False)) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2HandleAppReliable(GTI2Connection *connection,
                                     GT2Byte *message, int len) {
    // check the state
    if ((connection->state != GTI2Connected) &&
        (connection->state != GTI2Closing)) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
    } else if (ArrayLength(connection->receiveFilters)) {
        if (!gti2ReceiveFilterCallback(connection, 0, message, len, GT2True)) {
            return GT2False;
        }
        return GT2True;
    } else {
        if (!gti2ReceivedCallback(connection, message, len, GT2True)) {
            return GT2False;
        }
    }

    return GT2True;
}

static GT2Bool gti2HandleClientChallenge(GTI2Connection *connection,
                                         GT2Byte *message, int len) {
    char response[GTI2_RESPONSE_LEN];
    char challenge[GTI2_CHALLENGE_LEN];

    // check the state
    if (connection->state != GTI2AwaitingClientChallenge) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // check the length
    if (len < GTI2_CHALLENGE_LEN) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // get the response to the client's challenge
    gti2GetResponse((GT2Byte *)response, message);

    // get our challenge and the expected response
    gti2GetChallenge((GT2Byte *)challenge);
    gti2GetResponse((GT2Byte *)connection->response, (GT2Byte *)challenge);

    // send the challenge/response
    if (!gti2SendServerChallenge(connection, response, challenge)) {
        return GT2False;
    }

    connection->state = GTI2AwaitingClientResponse;

    return GT2True;
}

static GT2Bool gti2HandleServerChallenge(GTI2Connection *connection,
                                         GT2Byte *message, int len) {
    char response[GTI2_RESPONSE_LEN];

    // check the state
    if (connection->state != GTI2AwaitingServerChallenge) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // check the length
    if (len < (GTI2_RESPONSE_LEN + GTI2_CHALLENGE_LEN)) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // check the server's response to our challenge
    if (!gti2CheckResponse(message, (GT2Byte *)connection->response)) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // get the response to the server's challenge
    gti2GetResponse((GT2Byte *)response, message + GTI2_RESPONSE_LEN);

    // send the response
    if (!gti2SendClientResponse(connection, response,
                                connection->initialMessage,
                                connection->initialMessageLen)) {
        return GT2False;
    }

    // free the initial message
    if (connection->initialMessage) {
        gsifree(connection->initialMessage);
        connection->initialMessage = NULL;
    }

    connection->state = GTI2AwaitingAcceptance;

    return GT2True;
}

static GT2Bool gti2HandleClientResponse(GTI2Connection *connection,
                                        GT2Byte *message, int len) {
    int latency;

    // check the state
    if (connection->state != GTI2AwaitingClientResponse) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // check the length
    if (len < GTI2_RESPONSE_LEN) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // check the client's response
    if (!gti2CheckResponse(message, (GT2Byte *)connection->response)) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // if there's no callback, there's no one to accept the connection
    if (!connection->socket->connectAttemptCallback) {
        if (!gti2SendClosed(connection)) {
            return GT2False;
        }

        gti2ConnectionClosed(connection);

        return GT2True;
    }

    // the app needs to accept or reject
    connection->state = GTI2AwaitingAcceptReject;

    // get the latency
    latency = (int)(current_time() - connection->challengeTime);

    if (!gti2ConnectAttemptCallback(
            connection->socket, connection, connection->ip, connection->port,
            latency, message + GTI2_RESPONSE_LEN, len - GTI2_RESPONSE_LEN)) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2HandleAccept(GTI2Connection *connection) {
    // check the state
    if (connection->state != GTI2AwaitingAcceptance) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // we're connected
    connection->state = GTI2Connected;

    if (!gti2ConnectedCallback(connection, GT2Success, NULL, 0)) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2HandleReject(GTI2Connection *connection, GT2Byte *message,
                                int len) {
    // check the state
    if (connection->state != GTI2AwaitingAcceptance) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    gti2ConnectionClosed(connection);

    // let the other side know we got the reject
    if (!gti2SendClosed(connection)) {
        return GT2False;
    }

    if (!gti2ConnectedCallback(connection, GT2Rejected, message, len)) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2HandleClose(GTI2Connection *connection) {
    GT2Bool localClose;

    // let the other side know we got the close
    if (!gti2SendClosed(connection)) {
        return GT2False;
    }

    localClose = (connection->state == GTI2Closing);

    if (!gti2ConnectionError(connection, GT2Rejected,
                             localClose ? GT2LocalClose : GT2RemoteClose)) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2DeliverReliableMessage(GTI2Connection *connection,
                                          GTI2MessageType type,
                                          GT2Byte *message, int len) {
    // one more message was received
    connection->expectedSerialNumber++;

    if (type == GTI2MsgAppReliable) {
        if (!gti2HandleAppReliable(connection, message, len)) {
            return GT2False;
        }
    } else if (type == GTI2MsgClientChallenge) {
        if (!gti2HandleClientChallenge(connection, message, len)) {
            return GT2False;
        }
    } else if (type == GTI2MsgServerChallenge) {
        if (!gti2HandleServerChallenge(connection, message, len)) {
            return GT2False;
        }
    } else if (type == GTI2MsgClientResponse) {
        if (!gti2HandleClientResponse(connection, message, len)) {
            return GT2False;
        }
    } else if (type == GTI2MsgAccept) {
        if (!gti2HandleAccept(connection)) {
            return GT2False;
        }
    } else if (type == GTI2MsgReject) {
        if (!gti2HandleReject(connection, message, len)) {
            return GT2False;
        }
    } else if (type == GTI2MsgClose) {
        if (!gti2HandleClose(connection)) {
            return GT2False;
        }
    }

    return GT2True;
}

static int GS_STATIC_CALLBACK
gti2IncomingBufferMessageCompare(const void *elem1, const void *elem2) {
    const GTI2IncomingBufferMessage *message1 =
        (const GTI2IncomingBufferMessage *)elem1;
    const GTI2IncomingBufferMessage *message2 =
        (const GTI2IncomingBufferMessage *)elem2;

    return gti2SNDiff(message1->serialNumber, message2->serialNumber);
}

static GT2Bool gti2BufferIncomingMessage(GTI2Connection *connection,
                                         GTI2MessageType type,
                                         unsigned short SN, GT2Byte *message,
                                         int len, int *overflow) {
    GTI2IncomingBufferMessage messageInfo;
    GTI2IncomingBufferMessage *bufferedMessage;
    int num;
    int i;
    GTI2IncomingBufferMessage *msg;
    GTI2IncomingBufferMessage *prev;
    unsigned short diff;

    num = ArrayLength(connection->incomingBufferMessages);

    // look for the place for this message, and check for duplicates
    for (i = 0; i < num; i++) {
        bufferedMessage = (GTI2IncomingBufferMessage *)ArrayNth(
            connection->incomingBufferMessages, i);

        // check for a duplicate
        if (bufferedMessage->serialNumber == SN) {
            *overflow = GT2False;
            return GT2True;
        }

        // check if we've passed it
        if (gti2SNDiff(bufferedMessage->serialNumber, SN) > 0) {
            break;
        }
    }

    // check for overflow
    if (gti2GetBufferFreeSpace(&connection->incomingBuffer) < len) {
        *overflow = GT2True;
        return GT2True;
    }

    // fill in the message info
    messageInfo.start = connection->incomingBuffer.len;
    messageInfo.len = len;
    messageInfo.type = type;
    messageInfo.serialNumber = SN;

    // add it to the list
    ArrayInsertSorted(connection->incomingBufferMessages, &messageInfo,
                      gti2IncomingBufferMessageCompare);

    // check for overflow
    if (ArrayLength(connection->incomingBufferMessages) != (num + 1)) {
        *overflow = GT2True;
        return GT2True;
    }

    // copy the message into the buffer
    gti2BufferWriteData(&connection->incomingBuffer, message, len);

    // if it's first, send a nack for anything before it
    if (num == 0) {
        if (!gti2SendNack(connection, connection->expectedSerialNumber,
                          (unsigned short)(SN - 1))) {
            return GT2False;
        }
    } else {
        // check if it's a gap after the one before it
        msg = (GTI2IncomingBufferMessage *)ArrayNth(
            connection->incomingBufferMessages, num);
        if (msg->serialNumber == SN) {
            prev = (GTI2IncomingBufferMessage *)ArrayNth(
                connection->incomingBufferMessages, num - 1);
            diff = (unsigned short)(SN - prev->serialNumber);
            if (diff > 1) {
                if (!gti2SendNack(connection,
                                  (unsigned short)(prev->serialNumber + 1),
                                  (unsigned short)(SN - 1))) {
                    return GT2False;
                }
            }
        }
    }

    *overflow = GT2False;

    return GT2True;
}

static void gti2RemoveHoldMessage(GTI2Connection *connection,
                                  GTI2IncomingBufferMessage *message,
                                  int index) {
    int moveAfter;
    int shortenBy;
    int moveEnd;
    int num;
    int i;

    moveAfter = message->start;
    shortenBy = message->len;
    moveEnd = 0;

    ArrayDeleteAt(connection->incomingBufferMessages, index);

    // move the other messages' starts down
    num = ArrayLength(connection->incomingBufferMessages);
    for (i = 0; i < num; i++) {
        message = (GTI2IncomingBufferMessage *)ArrayNth(
            connection->incomingBufferMessages, i);
        if (message->start > moveAfter) {
            message->start -= shortenBy;
            moveEnd = moveEnd > message->start + message->len
                          ? moveEnd
                          : message->start + message->len;
        }
    }

    gti2BufferShorten(&connection->incomingBuffer, moveAfter, shortenBy);
}

static GT2Bool gti2DeliverHoldMessages(GTI2Connection *connection) {
    GTI2IncomingBufferMessage *message;
    int num;
    int i;

restart:
    num = ArrayLength(connection->incomingBufferMessages);
    for (i = num - 1; i >= 0; i--) {
        message = (GTI2IncomingBufferMessage *)ArrayNth(
            connection->incomingBufferMessages, i);

        if (message->serialNumber == connection->expectedSerialNumber) {
            // deliver it
            if (!gti2DeliverReliableMessage(connection, message->type,
                                            connection->incomingBuffer.buffer +
                                                message->start,
                                            message->len)) {
                return GT2False;
            }

            // remove it
            gti2RemoveHoldMessage(connection, message, i);

            goto restart;
        }
    }

    return GT2True;
}

static void gti2SetPendingAck(GTI2Connection *connection) {
    if (!connection->pendingAck) {
        connection->pendingAck = GT2True;
        connection->pendingAckTime = current_time();
    }
}

static GT2Bool gti2HandleReliableMessage(GTI2Connection *connection,
                                         GTI2MessageType type, GT2Byte *message,
                                         int len) {
    unsigned short SN;
    unsigned short ESN;
    int headerLength;
    int overflow;

    headerLength = connection->socket->protocolOffset + 7;

    // check the length
    if (len < headerLength) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // read the serial numbers
    SN = gti2UShortFromBuffer(message + connection->socket->protocolOffset, 3);
    ESN = gti2UShortFromBuffer(message + connection->socket->protocolOffset, 5);

    // keep the VDP header in front of application data
    if ((connection->socket->protocolType == GTI2VdpProtocol) &&
        (type == GTI2MsgAppReliable)) {
        message[connection->socket->protocolOffset + 5] = message[0];
        message[connection->socket->protocolOffset + 6] = message[1];
        message += headerLength - connection->socket->protocolOffset;
        len -= headerLength - connection->socket->protocolOffset;
    } else {
        message += headerLength;
        len -= headerLength;
    }

    // handle the ESN
    if (!gti2HandleESN(connection, ESN)) {
        return GT2False;
    }

    // check if this is the message we expect
    if (SN == connection->expectedSerialNumber) {
        // an ack is now needed
        gti2SetPendingAck(connection);

        // deliver it
        if (!gti2DeliverReliableMessage(connection, type, message, len)) {
            return GT2False;
        }

        // deliver any held messages that are now in order
        if (!gti2DeliverHoldMessages(connection)) {
            return GT2False;
        }

        return GT2True;
    }

    // check if it's an old message
    if (gti2SNDiff(SN, connection->expectedSerialNumber) < 0) {
        // an ack is needed so the sender knows we got it
        gti2SetPendingAck(connection);

        return GT2True;
    }

    // it's from the future, hold on to it
    if (!gti2BufferIncomingMessage(connection, type, SN, message, len,
                                   &overflow)) {
        return GT2False;
    }

    // if it didn't fit, close the connection
    if (overflow) {
        if (!gti2ConnectionMemoryError(connection)) {
            return GT2False;
        }
    }

    return GT2True;
}

static GT2Bool gti2HandleAck(GTI2Connection *connection, const GT2Byte *message,
                             int len) {
    unsigned short ESN;

    // check the length
    if (len != 2) {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // get the ESN
    ESN = gti2UShortFromBuffer(message, 0);

    if (!gti2HandleESN(connection, ESN)) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2HandleNack(GTI2Connection *connection,
                              const GT2Byte *message, int len) {
    unsigned short SNMin;
    unsigned short SNMax;
    int num;
    int i;
    GTI2OutgoingBufferMessage *messageInfo;

    // get the range of SNs
    SNMin = gti2UShortFromBuffer(message, 0);
    if (len == 2) {
        SNMax = SNMin;
    } else if (len == 4) {
        SNMax = gti2UShortFromBuffer(message, 2);
    } else {
        if (!gti2ConnectionCommunicationError(connection)) {
            return GT2False;
        }
        return GT2True;
    }

    // resend the messages in the range
    num = ArrayLength(connection->outgoingBufferMessages);
    for (i = 0; i < num; i++) {
        messageInfo = (GTI2OutgoingBufferMessage *)ArrayNth(
            connection->outgoingBufferMessages, i);

        if ((gti2SNDiff(messageInfo->serialNumber, SNMin) >= 0) &&
            (gti2SNDiff(messageInfo->serialNumber, SNMax) <= 0)) {
            if (!gti2ResendMessage(connection, messageInfo)) {
                return GT2False;
            }
        }
    }

    return GT2True;
}

static GT2Bool gti2HandlePing(GTI2Connection *connection, GT2Byte *message,
                              int len) {
    // turn the ping into a pong and send it back
    return gti2SendPong(connection, message, len);
}

static GT2Bool gti2HandlePong(GTI2Connection *connection,
                              const GT2Byte *message, int len) {
    unsigned int startTime;

    // ignore it if no one is listening
    if (!connection->callbacks.ping) {
        return GT2True;
    }

    // check the message
    if (len != (4 + sizeof(startTime))) {
        return GT2True;
    }
    if (memcmp(message, "time", 4)) {
        return GT2True;
    }

    // get the start time
    memcpy(&startTime, message + 4, 4);

    if (!gti2PingCallback(connection, (int)(current_time() - startTime))) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2HandleClosed(GTI2Connection *connection) {
    GT2Bool localClose;

    // the other side confirmed the close
    if (connection->state == GTI2Closed) {
        return GT2True;
    }

    localClose = (connection->state == GTI2Closing);

    if (!gti2ConnectionError(connection, GT2Rejected,
                             localClose ? GT2LocalClose : GT2RemoteClose)) {
        return GT2False;
    }
    return GT2True;
}

static GT2Bool gti2HandleUnreliableMessage(GTI2Connection *connection,
                                           GTI2MessageType type,
                                           GT2Byte *message, int len) {
    int headerLength;
    GT2Byte *dataStart;
    int dataLen;

    headerLength = connection->socket->protocolOffset + 3;
    dataStart = message + headerLength;
    dataLen = len - headerLength;

    if (type == GTI2MsgAck) {
        if (!gti2HandleAck(connection, dataStart, dataLen)) {
            return GT2False;
        }
    } else if (type == GTI2MsgNack) {
        if (!gti2HandleNack(connection, dataStart, dataLen)) {
            return GT2False;
        }
    } else if (type == GTI2MsgPing) {
        if (!gti2HandlePing(connection, message, len)) {
            return GT2False;
        }
    } else if (type == GTI2MsgPong) {
        if (!gti2HandlePong(connection, dataStart, dataLen)) {
            return GT2False;
        }
    } else if (type == GTI2MsgClosed) {
        if (!gti2HandleClosed(connection)) {
            return GT2False;
        }
    }

    return GT2True;
}

static GT2Bool gti2HandleMessage(GTI2Socket *socket, GT2Byte *message, int len,
                                 unsigned int ip, unsigned short port) {
    GTI2Connection *connection;
    GT2Bool magicString;
    GT2Result result;
    GTI2MessageType type;
    GT2Bool handled;
    int actualLength;
    GT2Byte *actualMessage;

    actualMessage = message + socket->protocolOffset;
    actualLength = len - socket->protocolOffset;

    // find the connection
    connection = gti2SocketFindConnection(socket, ip, port);

    // call the dump callback
    if (socket->receiveDumpCallback) {
        if (!gti2DumpCallback(socket, connection, ip, port, GT2False, message,
                              len, GT2False)) {
            return GT2False;
        }
    }

    // check for the magic string
    magicString = GT2False;
    if ((actualLength > GTI2_MAGIC_STRING_LEN) &&
        (memcmp(actualMessage, GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN) ==
         0)) {
        magicString = GT2True;
    }

    // check if this is from an unknown connection
    if (!connection) {
        // let the app handle it first
        if (!gti2UnrecognizedMessageCallback(socket, ip, port, message, len,
                                             &handled)) {
            return GT2False;
        }
        if (handled) {
            return GT2True;
        }

        // anything but a new connection attempt can't be handled
        if (!(magicString && (actualMessage[2] == GTI2MsgClientChallenge))) {
            // don't reply to a closed message with a closed message
            if (!magicString || (actualMessage[2] != GTI2MsgClosed)) {
                // let the sender know
                if (!gti2SendClosedOnSocket(socket, ip, port)) {
                    return GT2False;
                }
            }

            return GT2True;
        }

        // ignore it if we aren't listening
        if (!socket->connectAttemptCallback) {
            return GT2True;
        }

        // create the connection
        result = gti2NewIncomingConnection(socket, &connection, ip, port);
        if (result != GT2Success) {
            if (result != GT2DuplicateAddress) {
                // let the sender know
                if (!gti2SendClosedOnSocket(socket, ip, port)) {
                    return GT2False;
                }
            }

            return GT2True;
        }
    }

    // check if the connection is closed
    if (connection->state == GTI2Closed) {
        // tell the sender
        if (!magicString || (actualMessage[2] != GTI2MsgClosed)) {
            if (!gti2SendClosed(connection)) {
                return GT2False;
            }
        }

        return GT2True;
    }

    // check for an unreliable application message starting with the magic
    // string
    if (magicString && (actualLength >= (GTI2_MAGIC_STRING_LEN * 2)) &&
        (memcmp(actualMessage + GTI2_MAGIC_STRING_LEN, GTI2_MAGIC_STRING,
                GTI2_MAGIC_STRING_LEN) == 0)) {
        magicString = GT2False;

        // skip the first copy of the magic string
        message[3] = message[1];
        message[2] = message[0];
        message += GTI2_MAGIC_STRING_LEN;
        actualMessage += GTI2_MAGIC_STRING_LEN;
        len -= GTI2_MAGIC_STRING_LEN;
    }

    // application unreliable message
    if (!magicString) {
        // not connected yet, give it to the unrecognized message callback
        if (connection->state < GTI2Connected) {
            if (!gti2UnrecognizedMessageCallback(socket, ip, port, message, len,
                                                 &handled)) {
                return GT2False;
            }
        } else {
            if (!gti2HandleAppUnreliable(connection, message, len)) {
                return GT2False;
            }
        }

        return GT2True;
    }

    type = (GTI2MessageType)actualMessage[2];

    if (type < GTI2NumReliableMessages) {
        if (!gti2HandleReliableMessage(connection, type, message, len)) {
            return GT2False;
        }
        return GT2True;
    }

    if (!gti2HandleUnreliableMessage(connection, type, message, len)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2HandleConnectionReset(GTI2Socket *socket, unsigned int ip,
                                  unsigned short port) {
    GTI2Connection *connection;

    connection = gti2SocketFindConnection(socket, ip, port);

    if (socket->receiveDumpCallback) {
        if (!gti2DumpCallback(socket, connection, ip, port, GT2True, NULL, 0,
                              GT2False)) {
            return GT2False;
        }
    }

    if (!connection) {
        return GT2True;
    }

    if (connection->state == GTI2AwaitingServerChallenge) {
        // check the timeout
        if (!connection->timeout ||
            ((current_time() - connection->startTime) < connection->timeout)) {
            return GT2True;
        }

        if (!gti2ConnectionError(connection, GT2TimedOut, GT2RemoteClose)) {
            return GT2False;
        }
    } else {
        if (!gti2ConnectionError(connection, GT2Rejected, GT2RemoteClose)) {
            return GT2False;
        }
    }

    return GT2True;
}

GT2Bool gti2HandleHostUnreachable(GTI2Socket *socket, unsigned int ip,
                                  unsigned short port, GT2Bool send) {
    GTI2Connection *connection;

    connection = gti2SocketFindConnection(socket, ip, port);

    if (socket->receiveDumpCallback) {
        if (!gti2DumpCallback(socket, connection, ip, port, GT2True, NULL, 0,
                              send)) {
            return GT2False;
        }
    }

    if (!connection) {
        return GT2True;
    }

    if (!gti2ConnectionError(connection, GT2TimedOut, GT2RemoteClose)) {
        return GT2False;
    }
    return GT2True;
}

static char buffer[4096];

GT2Bool gti2ReceiveMessages(GTI2Socket *socket) {
    int rcode;
    SOCKADDR_IN address;
    int addressLen;

    while (CanReceiveOnSocket(socket->socket)) {
        addressLen = sizeof(SOCKADDR_IN);
        rcode = recvfrom(socket->socket, buffer, sizeof(buffer), 0,
                         (SOCKADDR *)&address, &addressLen);
        if (gsiSocketIsError(rcode)) {
            rcode = GOAGetLastError(socket->socket);
            if (rcode == WSAECONNRESET) {
                if (!gti2HandleConnectionReset(socket, address.sin_addr.s_addr,
                                               ntohs(address.sin_port))) {
                    return GT2False;
                }
            } else if (rcode == WSAEHOSTUNREACH) {
                if (!gti2HandleHostUnreachable(socket, address.sin_addr.s_addr,
                                               ntohs(address.sin_port),
                                               GT2False)) {
                    return GT2False;
                }
            } else if (rcode != WSAEMSGSIZE) {
                gti2SocketError(socket);
                return GT2False;
            }
        } else {
            if (!gti2HandleMessage((GTI2Socket *)socket, (GT2Byte *)buffer,
                                   rcode, address.sin_addr.s_addr,
                                   ntohs(address.sin_port))) {
                return GT2False;
            }
        }
    }

    return GT2True;
}

static GT2Bool gti2StoreOutgoingReliableMessageInfo(GTI2Connection *connection,
                                                    unsigned short SN,
                                                    int len) {
    GTI2OutgoingBufferMessage messageInfo;
    int num;

    memset(&messageInfo, 0, sizeof(messageInfo));
    messageInfo.start = connection->outgoingBuffer.len;
    messageInfo.len = len;
    messageInfo.serialNumber = SN;
    messageInfo.lastSend = current_time();

    num = ArrayLength(connection->outgoingBufferMessages);
    ArrayAppend(connection->outgoingBufferMessages, &messageInfo);

    return (ArrayLength(connection->outgoingBufferMessages) == (num + 1));
}

static GT2Bool gti2BeginReliableMessage(GTI2Connection *connection,
                                        GTI2MessageType type, int len,
                                        int *overflow) {
    int freeSpace;
    unsigned short vdpDataLength;

    vdpDataLength = (unsigned short)(len - connection->socket->protocolOffset);

    // check for overflow
    freeSpace = gti2GetBufferFreeSpace(&connection->outgoingBuffer);
    if (freeSpace < len) {
        if (!gti2ConnectionMemoryError(connection)) {
            return GT2False;
        }

        *overflow = GT2True;
        return GT2True;
    }

    // store the message info
    if (!gti2StoreOutgoingReliableMessageInfo(connection,
                                              connection->serialNumber, len)) {
        if (!gti2ConnectionMemoryError(connection)) {
            return GT2False;
        }

        *overflow = GT2True;
        return GT2True;
    }

    // write the header
    if (connection->socket->protocolType == GTI2VdpProtocol) {
        gti2BufferWriteData(&connection->outgoingBuffer,
                            (GT2Byte *)&vdpDataLength,
                            connection->socket->protocolOffset);
    }
    gti2BufferWriteData(&connection->outgoingBuffer,
                        (GT2Byte *)GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
    gti2BufferWriteByte(&connection->outgoingBuffer, (GT2Byte)type);
    gti2BufferWriteUShort(&connection->outgoingBuffer,
                          connection->serialNumber++);
    gti2BufferWriteUShort(&connection->outgoingBuffer,
                          connection->expectedSerialNumber);

    *overflow = GT2False;

    return GT2True;
}

static GT2Bool gti2EndReliableMessage(GTI2Connection *connection) {
    GTI2OutgoingBufferMessage *message;
    int len;

    // get the message that was just added
    len = ArrayLength(connection->outgoingBufferMessages);
    message = (GTI2OutgoingBufferMessage *)ArrayNth(
        connection->outgoingBufferMessages, len - 1);

    // send it
    if (!gti2ConnectionSendData(
            connection, connection->outgoingBuffer.buffer + message->start,
            message->len)) {
        return GT2False;
    }

    // the ack went with it
    connection->pendingAck = GT2False;

    return GT2True;
}

GT2Bool gti2SendAppReliable(GTI2Connection *connection, const GT2Byte *message,
                            int len) {
    int totalLen;
    int overflow;

    totalLen = len + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgAppReliable, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    gti2BufferWriteData(&connection->outgoingBuffer, message, len);

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendClientChallenge(GTI2Connection *connection,
                                const char challenge[GTI2_CHALLENGE_LEN]) {
    int totalLen;
    int overflow;

    totalLen = connection->socket->protocolOffset + GTI2_CHALLENGE_LEN + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgClientChallenge, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)challenge,
                        GTI2_CHALLENGE_LEN);

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendServerChallenge(GTI2Connection *connection,
                                const char response[GTI2_RESPONSE_LEN],
                                const char challenge[GTI2_CHALLENGE_LEN]) {
    int totalLen;
    int overflow;

    totalLen = connection->socket->protocolOffset + GTI2_RESPONSE_LEN +
               GTI2_CHALLENGE_LEN + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgServerChallenge, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)response,
                        GTI2_RESPONSE_LEN);
    gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)challenge,
                        GTI2_CHALLENGE_LEN);

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    connection->challengeTime = connection->lastSend;
    return GT2True;
}

GT2Bool gti2SendClientResponse(GTI2Connection *connection,
                               const char response[GTI2_RESPONSE_LEN],
                               const char *message, int len) {
    int totalLen;
    int overflow;

    totalLen = connection->socket->protocolOffset + GTI2_RESPONSE_LEN + len + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgClientResponse, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)response,
                        GTI2_RESPONSE_LEN);
    gti2BufferWriteData(&connection->outgoingBuffer, (const GT2Byte *)message,
                        len);

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendAccept(GTI2Connection *connection) {
    int totalLen;
    int overflow;

    totalLen = connection->socket->protocolOffset + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgAccept, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendReject(GTI2Connection *connection, const GT2Byte *message,
                       int len) {
    int totalLen;
    int overflow;

    totalLen = connection->socket->protocolOffset + len + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgReject, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    gti2BufferWriteData(&connection->outgoingBuffer, message, len);

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendClose(GTI2Connection *connection) {
    int totalLen;
    int overflow;

    totalLen = connection->socket->protocolOffset + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgClose, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendKeepAlive(GTI2Connection *connection) {
    int totalLen;
    int overflow;

    totalLen = connection->socket->protocolOffset + 7;

    if (!gti2BeginReliableMessage(connection, GTI2MsgKeepAlive, totalLen,
                                  &overflow)) {
        return GT2False;
    }

    if (overflow) {
        return GT2True;
    }

    if (!gti2EndReliableMessage(connection)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendAppUnreliable(GTI2Connection *connection,
                              const GT2Byte *message, int len) {
    int freeSpace;
    int totalLen;
    GT2Byte *start;

    // check if the message starts with the magic string
    if ((len < GTI2_MAGIC_STRING_LEN) ||
        memcmp(message + connection->socket->protocolOffset, GTI2_MAGIC_STRING,
               GTI2_MAGIC_STRING_LEN)) {
        // send it as is
        if (!gti2ConnectionSendData(connection, message, len)) {
            return GT2False;
        }

        return GT2True;
    }

    // it needs an extra copy of the magic string
    totalLen = len + GTI2_MAGIC_STRING_LEN;

    freeSpace = gti2GetBufferFreeSpace(&connection->outgoingBuffer);
    if (freeSpace < totalLen) {
        return GT2True;
    }

    start = connection->outgoingBuffer.buffer + connection->outgoingBuffer.len;

    if (connection->socket->protocolType == GTI2VdpProtocol) {
        gti2BufferWriteData(&connection->outgoingBuffer, message,
                            MAX_PROTOCOL_OFFSET);
    }
    gti2BufferWriteData(&connection->outgoingBuffer,
                        (GT2Byte *)GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
    gti2BufferWriteData(&connection->outgoingBuffer,
                        message + connection->socket->protocolOffset,
                        len - connection->socket->protocolOffset);

    if (!gti2ConnectionSendData(connection, start, totalLen)) {
        return GT2False;
    }

    // remove it from the buffer
    gti2BufferShorten(&connection->outgoingBuffer, -1, totalLen);

    return GT2True;
}

GT2Bool gti2SendAck(GTI2Connection *connection) {
    char buffer[7];
    int pos;
    signed short dataLength;

    pos = 0;

    if (connection->socket->protocolType == GTI2VdpProtocol) {
        dataLength = 5;
        memcpy(buffer, &dataLength, 2);
        pos = 2;
    }

    memcpy(&buffer[pos], GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
    pos += GTI2_MAGIC_STRING_LEN;

    buffer[pos++] = GTI2MsgAck;
    gti2UShortToBuffer((GT2Byte *)buffer, pos,
                       connection->expectedSerialNumber);
    pos += 2;

    if (!gti2ConnectionSendData(connection, (GT2Byte *)buffer, pos)) {
        return GT2False;
    }

    connection->pendingAck = GT2False;

    return GT2True;
}

GT2Bool gti2SendNack(GTI2Connection *connection, unsigned short SNMin,
                     unsigned short SNMax) {
    char buffer[9];
    int pos;
    signed short dataLength;

    pos = 0;

    if (connection->socket->protocolType == GTI2VdpProtocol) {
        dataLength = 7;
        memcpy(buffer, &dataLength, 2);
        pos = 2;
    }

    memcpy(&buffer[pos], GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
    pos += GTI2_MAGIC_STRING_LEN;

    buffer[pos++] = GTI2MsgNack;
    gti2UShortToBuffer((GT2Byte *)buffer, pos, SNMin);
    pos += 2;

    if (SNMin != SNMax) {
        gti2UShortToBuffer((GT2Byte *)buffer, pos, SNMax);
        pos += 2;
    }

    if (!gti2ConnectionSendData(connection, (GT2Byte *)buffer, pos)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendPing(GTI2Connection *connection) {
    char buffer[11];
    int pos;
    signed short dataLength;
    unsigned int startTime;

    pos = 0;

    if (connection->socket->protocolType == GTI2VdpProtocol) {
        dataLength = 11;
        memcpy(buffer, &dataLength, 2);
        pos = 2;
    }

    memcpy(&buffer[pos], GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
    pos += GTI2_MAGIC_STRING_LEN;

    buffer[pos++] = GTI2MsgPing;
    memcpy(&buffer[pos], "time", 4);
    startTime = current_time();
    memcpy(&buffer[pos + 4], &startTime, 4);

    if (!gti2ConnectionSendData(connection, (GT2Byte *)buffer, pos + 8)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2SendPong(GTI2Connection *connection, GT2Byte *message, int len) {
    // change the ping into a pong
    message[GTI2_MAGIC_STRING_LEN] = GTI2MsgPong;

    return gti2ConnectionSendData(connection, message, len);
}

GT2Bool gti2SendClosed(GTI2Connection *connection) {
    return gti2SendClosedOnSocket(connection->socket, connection->ip,
                                  connection->port);
}

GT2Bool gti2SendClosedOnSocket(GTI2Socket *socket, unsigned int ip,
                               unsigned short port) {
    char buffer[5];
    int pos;
    signed short dataLength;

    pos = 0;

    if (socket->protocolType == GTI2VdpProtocol) {
        dataLength = 3;
        memcpy(buffer, &dataLength, 2);
        pos = 2;
    }

    memcpy(&buffer[pos], GTI2_MAGIC_STRING, GTI2_MAGIC_STRING_LEN);
    pos += GTI2_MAGIC_STRING_LEN;

    buffer[pos++] = GTI2MsgClosed;

    if (!gti2SocketSend(socket, ip, port, (GT2Byte *)buffer, pos)) {
        return GT2False;
    }
    return GT2True;
}

GT2Bool gti2ResendMessage(GTI2Connection *connection,
                          GTI2OutgoingBufferMessage *message) {
    GTI2MessageType type;
    int pos;

    // update the ESN in the message
    pos = message->start + connection->socket->protocolOffset;
    pos += 5;
    gti2UShortToBuffer(connection->outgoingBuffer.buffer, pos,
                       connection->expectedSerialNumber);

    // send it
    if (!gti2ConnectionSendData(
            connection, connection->outgoingBuffer.buffer + message->start,
            message->len)) {
        return GT2False;
    }

    message->lastSend = connection->lastSend;

    // check for a server challenge, which the timeout is based on
    type = (GTI2MessageType)connection->outgoingBuffer
               .buffer[message->start + connection->socket->protocolOffset + 2];
    if (type == GTI2MsgServerChallenge) {
        connection->challengeTime = connection->lastSend;
    }

    return GT2True;
}

GT2Bool gti2Send(GTI2Connection *connection, const GT2Byte *message, int len,
                 GT2Bool reliable) {
    if (reliable) {
        return gti2SendAppReliable(connection, message, len);
    }

    return gti2SendAppUnreliable(connection, message, len);
}
