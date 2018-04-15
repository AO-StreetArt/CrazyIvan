/*
Apache2 License Notice
Copyright 2017 Alex Barry

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "include/globals.h"

// Declare our global variables

// Response Object
SceneListInterface *resp = NULL;

// Set up an Scene pointer to hold the currently translated document information
SceneListInterface *translated_object = NULL;

// UUID Generator
uuidInterface *ua = NULL;

// Configuration Manager
ConfigurationManager *cm = NULL;

// Global Couchbase Admin Object
Neo4jInterface *neo = NULL;

// Command Line interpreter
CommandLineInterface *cli = NULL;

// Document Manager, highest level object
ProcessorInterface *processor = NULL;

// ZMQ Admins
Zmqio *zmqi = NULL;

// Factories
CommandLineInterpreterFactory *cli_factory = NULL;
Neo4jComponentFactory *neo4j_factory = NULL;
uuidComponentFactory *uuid_factory = NULL;
ZmqComponentFactory *zmq_factory = NULL;
LoggingComponentFactory *logging_factory = NULL;

ProcessorFactory *processor_factory = NULL;
SceneListFactory *scene_list_factory = NULL;
SceneFactory *scene_factory = NULL;
