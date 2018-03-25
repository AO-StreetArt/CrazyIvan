# The basic CrazyIvan Flow performs a series of basic checks on the CrazyIvan API
# It walks a single scene through a full flow, step by step, and validates
# all of the fields on the scene, transforms, and devices at each step of the way.

import zmq
import logging
import sys
import json
import copy

# Basic Config
log_file = 'basicIvanFlow.log'
log_level = logging.DEBUG

# Object data represented through each piece of the flow
test_data = {
    "key":"",
    "name":"basicFlowName",
    "latitude":124.0,
    "longitude":122.0,
    "distance":100.0,
    "region":"basicFlowRegion",
    "asset_ids":["basicAsset"],
    "tags":["basicTag"]
}

updated_test_data = {
    "key":"",
    "name":"updatedName",
    "latitude":128.0,
    "longitude":112.0,
    "distance":100.0,
    "region":"updatedRegion",
    "asset_ids":["updatedAsset"],
    "tags":["upatedTag"]
}

removal_test_data = {
    "name":"updatedName",
    "asset_ids":["basicAsset"],
    "tags":["basicTag"]
}

# Validation Methods
def parse_response(response):
    logging.debug("Parsing Response: %s" % response)
    parsed_json = None
    try:
        parsed_json = json.loads(response)
    except Exception as e:
        try:
            parsed_json = json.loads(response[1:])
        except Exception as e:
            logging.error('Unable to parse response')
            logging.error(e)
            logging.error(get_exception())
    return parsed_json

def validate_success(response):
    logging.debug("Validating Success Response")
    parsed_json = parse_response(response)

    if parsed_json is not None:
        assert(parsed_json["err_code"] == 100)

def validate_create_response(create_response, test_data):
    logging.debug("Validating Create Response")
    logging.debug("Validating against: %s" % test_data)
    parsed_json = parse_response(create_response)

    if parsed_json is not None:
        assert(parsed_json["err_code"] == 100)
        assert(parsed_json["num_records"] == 1)
        assert("key" in parsed_json["scenes"][0])
        test_data["key"] = parsed_json["scenes"][0]["key"]
        logging.info("Key set: %s" % test_data["key"])

def validate_get_response(get_response, validation_data):
    logging.debug("Validating Get Response")
    logging.debug("Validating against: %s" % validation_data)
    parsed_json = parse_response(get_response)

    if parsed_json is not None:
        assert(parsed_json["err_code"] == 100)
        assert(parsed_json["num_records"] == 1)
        parsed_data = parsed_json["scenes"][0]
        assert(parsed_data["key"] == validation_data["key"])
        assert(parsed_data["name"] == validation_data["name"])
        assert(parsed_data["latitude"] - validation_data["latitude"] < 0.01)
        assert(parsed_data["longitude"] - validation_data["longitude"] < 0.01)
        assert(parsed_data["distance"] - validation_data["distance"] < 0.01)
        assert(parsed_data["region"] == validation_data["region"])
        assert(len(parsed_data['asset_ids']) > 0)
        assert(len(parsed_data['tags']) > 0)
        for i in range(0, len(parsed_data['asset_ids'])):
            assert(parsed_data["asset_ids"][i] == validation_data["asset_ids"][i])
        for i in range(0, len(parsed_data['tags'])):
            assert(parsed_data["tags"][i] == validation_data["tags"][i])

def validate_update_response(update_response):
    logging.info("Validating Update Response")
    parsed_json = parse_response(update_response)

    if parsed_json is not None:
        assert(parsed_json["err_code"] == 100)
        assert(parsed_json["num_records"] == 1)

# Tests
def base_test(socket, test_data, msg_type, op_type=None):
    msg_data = {
        "msg_type": msg_type,
        "scenes": [test_data]
    }
    if op_type is not None:
        msg_data['operation'] = op_type
    message = json.dumps(msg_data)
    logging.debug(message)
    socket.send_string(message + "\n")
    response = socket.recv_string()
    logging.debug(response)
    return response

def create_test(socket, test_data):
    logging.info("Create Test")
    create_response = base_test(socket, test_data, 0)
    validate_create_response(create_response, test_data)

def get_test(socket, test_data):
    logging.info("Get Test")
    get_response = base_test(socket, {"key":test_data["key"]}, 2)
    validate_get_response(get_response, test_data)

def update_test(socket, test_data):
    logging.info("Update Test")
    update_response = base_test(socket, test_data, 1)
    validate_update_response(update_response)

def delete_test(socket, test_data):
    logging.info("Delete Test")
    delete_response = base_test(socket, {"key":test_data["key"]}, 3)
    validate_success(delete_response)

def update_removal_test(socket, test_data):
    logging.info("Update Test")
    remove_response = base_test(socket, removal_test_data, 1, op_type=11)
    validate_update_response(remove_response)

# CRUD Flow
def execute_crud_flow(socket, test_data, updated_test_data):
    logging.debug("Executing CRUD Flow")
    create_test(socket, test_data)
    get_test(socket, test_data)
    updated_test_data["key"] = test_data["key"]
    update_test(socket, updated_test_data)
    updated_test_data['asset_ids'].insert(0, test_data['asset_ids'][0])
    updated_test_data['tags'].insert(0, test_data['tags'][0])
    get_test(socket, updated_test_data)
    removal_test_data['key'] = test_data['key']
    update_removal_test(socket, removal_test_data)
    del updated_test_data['asset_ids'][0]
    del updated_test_data['tags'][0]
    get_test(socket, updated_test_data)
    delete_test(socket, updated_test_data)

# Registration Flow
def execute_registration_flow(socket, test_data, updated_test_data):
    logging.debug("Executing Registration Flow")

# Execute the actual tests
def execute_main(zmq_addr="tcp://localhost:5555"):
    # Grab the global pieces of data
    global test_data
    global updated_test_data
    global log_file
    global log_level

    logging.basicConfig(filename=log_file, level=log_level)

    # Connect to ZeroMQ
    context = zmq.Context()
    context.setsockopt(zmq.RCVTIMEO, 10000)
    context.setsockopt(zmq.LINGER, 0)
    socket = context.socket(zmq.REQ)
    socket.connect(zmq_addr)
    logging.debug("Connected to ZMQ Socket")

    # Execute each test with a deep copy of the data, so that it stays
    # independent through each test
    execute_crud_flow(socket,
                      copy.deepcopy(test_data),
                      copy.deepcopy(updated_test_data))
    execute_registration_flow(socket,
                      copy.deepcopy(test_data),
                      copy.deepcopy(updated_test_data))


if __name__ == "__main__":
    if len(sys.argv) > 1:
        execute_main(sys.argv[1])
    else:
        execute_main()
