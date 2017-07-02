#!/usr/bin/env python

import zmq
import sys

if __name__ == "__main__":

    # Determine where we are sending the message
    conn_str = ""
    if len(sys.argv) > 2:
        conn_str = "tcp://%s:%s" % (sys.argv[1], sys.argv[2])
    elif len(sys.argv) == 2:
        props_file_name = sys.argv[1]

        #Read props file and pull 0mq string
        with open(props_file_name) as f:
            hostname = ""
            port = ""
            for line in f:
                if not (len(line) == 0 or line.startswith("//") or line.startswith("#") or line.startswith("-")):
                    kv_list = line.split("=")
                    if kv_list[0] == "0MQ_Hostname":
                        hostname = kv_list[1].strip()
                        print("Connection Host (%s)" % conn_str)
                    elif kv_list[0] == "0MQ_Port":
                        port = kv_list[1].strip()
                        print("Connection Port (%s)" % conn_str)
            conn_str = "tcp://%s:%s" % (hostname, port)

    else:
        print("Invalid input parameters")

    # Actually send the message
    context = zmq.Context()

    # Socket to talk to server
    print("Connecting to crazy ivan")
    socket = context.socket(zmq.REQ)
    socket.connect(conn_str)

    # Send the request
    request = "{\"msg_type\": 999}"
    print("Sending request %s" % request)
    socket.send_string(request)

    # Get the reply.
    message = socket.recv()
    print("Received reply %s [ %s ]" % (request, message))
