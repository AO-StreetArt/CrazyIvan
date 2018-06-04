#!/bin/bash

# Generate CA key & certificate
openssl req -x509 -newkey rsa:4096 -keyout caKey.key -out caCert.pem -days 365

# Generate Client key & certificate signing request
openssl genrsa -out clientKey.key 2048
openssl req -new -key clientKey.key -out clientCert.csr

# Generate Client Certificate in crt format
openssl x509 -req -in clientCert.csr -CA caCert.pem -CAkey caKey.key -CAcreateserial -out MyClient1.crt -days 1024 -sha256
