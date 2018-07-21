..  http:example:: curl wget httpie python-requests

    POST /v1/deregister HTTP/1.1
    Host: localhost:5885
    Content-Type: application/json

    {
      "scenes":[
        {
          "key":"jklmnop",
          "devices":[
            {
              "key":"Ud132"
            }
          ]
        }
      ]
    }
