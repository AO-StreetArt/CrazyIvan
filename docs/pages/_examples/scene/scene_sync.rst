..  http:example:: curl wget httpie python-requests

    POST /v1/align HTTP/1.1
    Host: localhost:5885
    Content-Type: application/json

    {
      "scenes":[
        {
          "key":"jklmnop",
          "devices":[
            {
              "key":"Ud\_132",
              "transform":{
                "translation":[0.0,0.0,0.0],
                "rotation":[0.0,0.0,0.0]
              }
            }
          ]
        }
      ]
    }
