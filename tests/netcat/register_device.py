# Python script which takes the IP & socket of the computer as
# a command line argument, sends a Scene Create request
# and a Scene Register request to Crazy Ivan.  Finally, it
# injects the value into the udp_message.txt file, outputting
# a new file which can be sent over UDP

import sys
import requests


# Send a post request to Crazy Ivan, return the key of the scene from response
def send_http_request(url, postData):
    r = requests.post(url, data=postData)
    response_json = r.json()
    return response_json['scenes'][0]['key']


# Execute the central script logic
def execute_main(hostname, port):
    print("Starting UDP Test Generation -- Python")
    # Other test file locationss
    create_file = "scene_create.json"
    register_file = "scene_registration.json"
    udp_file = "udp_message.txt"
    udp_out_file = "udp_message_upd.txt"
    create_url = "http://localhost:8766/v1/scene"
    register_url = "http://localhost:8766/v1/register"

    # Variables to be filled in with values from files
    create_message = ""
    register_message = ""
    udp_message = ""
    scene_key = ""

    print("Sending HTTP Requests -- Python")

    # Send HTTP Messages
    with open(create_file) as c_file:
        create_message = c_file.read()
        scene_key = send_http_request(create_url, create_message)
        print("Scene Key: %s" % scene_key)

    with open(register_file) as r_file:
        register_message = r_file.read()
        register_message = register_message.replace("_SCENE_KEY_", "%s" % scene_key)
        register_message = register_message.replace("_HOSTNAME_", "%s" % hostname)
        register_message = register_message.replace("_PORT_", "%s" % port)
        send_http_request(register_url, register_message)

    # Add the scene to the cache
    requests.put("http://localhost:8766/v1/scene/cache/" + scene_key)

    print("Generating UDP Message -- Python")

    # Build UDP Message
    with open(udp_file, "r") as u_file:
        udp_message = u_file.read()
        udp_message = udp_message.replace("_SCENE_KEY_", "%s" % scene_key)

    with open(udp_out_file, "w") as w_file:
        w_file.write(udp_message)

    print("Finishing UDP Test Generation -- Python")


if __name__ == "__main__":
    if (len(sys.argv) == 2):
        print(sys.argv[1])
        reload(sys)
        sys.setdefaultencoding('utf8')
        execute_main(sys.argv[1], 5005)
    else:
        print(len(sys.argv))
