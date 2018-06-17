# Python script which takes the IP & socket of the computer as
# a command line argument, sends a Scene Create request
# and a Scene Register request to Crazy Ivan.  Finally, it
# injects the value into the udp_message.txt file, outputting
# a new file which can be sent over UDP

import sys
import requests


# Execute the central script logic
def execute_main(scene_key):
    print("Removing scene from cache -- Python")

    # Add the scene to the cache
    requests.delete("http://localhost:8766/v1/scene/cache/" + scene_key)


if __name__ == "__main__":
    if (len(sys.argv) == 2):
        print(sys.argv[1])
        reload(sys)
        sys.setdefaultencoding('utf8')
        execute_main(sys.argv[1])
    else:
        print(len(sys.argv))
