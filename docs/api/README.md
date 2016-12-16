Data Storage API Design Document

Author – Alex Barry

<span id="anchor"></span>Section 1: Abstract
============================================

This document outlines the design for the Data Storage Service
API. This service stores potentially large-scale resources for use in distributed
visualizations. The design proposed in this document follows a series of principles:

-   Messages should require only the fields necessary.
-   Where intelligent updates incur performance penalties, they should
    be configurable and scalable. Therefor, message behavior may depend
    on server side configurations. The accepted/expected fields in
    messages, however, will not change.

<span id="anchor-1"></span>Section 2: Objectives
================================================

-   The primary purpose of this document is to detail the API for the
    Data Storage Service, Ex38.
-   Messaging will be via Protocol Buffer format
    with 0MQ as the message transport layer.

<span id="anchor-2"></span>Section 3: Inbound Messages
======================================================
| Field Name | Description | Data Type | Required? | | | |
| ---------- | ----------- | --------- | --------- | --- | --- | --- |
| | | | Create | Retrieve | Update | Destroy |
|  |  |  |  |  |  |  |
| message_type | <ul><li>OBJ_UPD=0</li><li>OBJ_CRT=1</li><li>OBJ_GET=2</li><li>OBJ_DEL=3</li><li>OBJ_LOCK=5</li><li>OBJ_UNLOCK=6</li><li>KILL=999</li><li>PING=555</li></ul> | Integer | X | X | X | X |
| key | The UUID of the object | String | * | X | X | X |
| data_chunk | The data being transferred | String | X | | * | |
| file_type | The file type of the string being stored, ie. "obj" | String | X | | * | |
| err_msg | If an error is encountered, it will be returned in this field | String | X | | * | |
| chunk_ids | The UUID's of the chunks corresponding to the header record | Array - String | X | | * | |
| scenes | The list of string scene UUID's | Array - String | X | | * | * |
| lock_device_id | The UUID of the locking User Device. | String | X | | * | |
| obj3_id | The UUID of the obj3 that corresponds to this object. | String | X | | * | |
| transaction_id | The UUID of the current transaction. | String | | | | |

X – Required

\* - Potentially Required for message to cause any action, depending on
configuration

<span id="anchor-3"></span>Object Create
----------------------------------------

The inbound object create message requires all fields to construct the
initial object, except for the lock\_device\_id and owner\_device\_id.
These are only required if the respective functionality is configured.

Note that if a key is included, then this is treated as an 'append' operation.
The data in the message is added to the existing data.  If a key is included and
no data is included, then the header information will be updated.

<span id="anchor-4"></span>Object Retrieve
------------------------------------------

The object retrieve message only requires a device UUID and message
type.

<span id="anchor-5"></span>Object Update
----------------------------------------

The behavior of the object update message triggers an overwrite of the existing data records.

<span id="anchor-6"></span>Object Destroy
-----------------------------------------

For the object destroy message, the message\_type and key fields are
required.

<span id="anchor-6a"></span>Object Lock
-----------------------------------------

If configured, locking on an object requires a key, message type, and lock_device_id.  This sends a lock request, which may be accepted or denied.

<span id="anchor-6b"></span>Object Unlock
-----------------------------------------

If configured, unlocking an object requires a key, message type, and lock_device_id.  This sends an unlock request, which may be accepted or denied.

<span id="anchor-7"></span>Section 4: Inbound Responses
=======================================================
| Field Name | Description | Data Type | Included? | | | |
| ---------- | ----------- | --------- | --------- | --- | --- | --- |
| | | | Create | Retrieve | Update | Destroy |
|  |  |  |  |  |  |  |
| message_type | <ul><li>NO_ERROR=0</li><li>ERROR=100</li><li>DB_ERROR=110</li><li>REDIS_ERROR=120</li><li>TRANSLATION_ERROR=130</li><li>BAD_SERVER_ERROR=140</li><li>BAD_REQUEST_ERROR=150</li></ul> | Integer | * | * | * | * |
| err_msg | A description of any error that occurred | String | * | * | * | * |
| key | The key of the object | String | X | X | X | X |
| transaction_id | The Transaction ID.  This is the same as the transaction ID given on the inbound response and can be used to link the two together, if this functionality is configured. | String | * | * | * | * |

X – Guaranteed

\* - Potentially Included, depending on whether we have a success or
failure response and/or configuration

Note: Our responses will also be in the form of the DataObject.proto messages.  
In the case of a Retrieval Response, the full object will be returned.
