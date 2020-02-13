var http = require("http"),
  request = require('request'),
  fs = require('fs'),
  url = require('url'),  
  async = require('async'),
  WebSocket = require('ws');

var portServer = 80;
var portWebSocketClient = 81;
var portWebSocketServer = 82;
var serviceRootUrl = '192.168.0.101';

server = http.createServer(function (req, res) {
  console.log('Nowe żądanie od klienta...');
   if (req.url === '/') {  
            fs.readFile(__dirname + '/html/index.html', function(err, data) {  
                if (err) {  
                    res.writeHead(404);  
                    res.write(err);  
                    res.end();  
                } else {  
                    res.writeHead(200, {  
                        'Content-Type': 'text/html'  
                    });  
                    res.write(data);  
                    res.end();  
                }
			});				
   }
  else if (req.url === '/get_temp') {
		async.series([ 
        function (callback) {
          request({url: 'http://' + serviceRootUrl + '/sensors/dth', json: true}, function (err, res, body) {
            if (err) callback(err);
            if (res && res.statusCode === 200) {
              console.log(body);
              var temp = body.temperature;
              callback(null, temp); 
            } else callback(null, null);
          });
        }],
      function (err, results) {
        console.log(results); 
        var logEntry = results[0] + '';
        fs.appendFile('log.txt', logEntry + '\n', encoding = 'utf8', function (err) {
          if (err) throw err;
          res.writeHeader(200, {"Content-Type": "text/plain"});
          res.write(logEntry);
          res.end();
        });
      });
  } else if (req.url === '/get_hum') {
		async.series([ 
        function (callback) {
          request({url: 'http://' + serviceRootUrl + '/sensors/dth', json: true}, function (err, res, body) {
            if (err) callback(err);
            if (res && res.statusCode === 200) {
              console.log(body);
              var hum = body.humidity;
              callback(null, hum); 
            } else callback(null, null);
          });
        }],
      function (err, results) { 
        console.log(results);  
        var logEntry = results[0] + '';
        fs.appendFile('log.txt', logEntry + '\n', encoding = 'utf8', function (err) {
          if (err) throw err;
          res.writeHeader(200, {"Content-Type": "text/plain"});
          res.write(logEntry);
          res.end();
        });
      });
	} else if (req.url === '/lights_on') {
		var options = {
		  url: 'http://' + serviceRootUrl + '/actuators/relay', 
		  headers: {"content-type": "application/json",},
		  json: true,
		  method: 'PUT',
		  body: 'I am an attachment',
		  json: {
			  "id": 1,
			  "gpio": 2,
			  "status": 0,
		  }
		};
		request(options, function (err, res, body) {
			if (err) throw (err);
			if (res && res.statusCode === 200) {
				console.log('Json przesłany do serwera!');
			}
		});
		res.writeHeader(200, {"Content-Type": "text/plain"});
		res.write('Json przesłany do serwera!');
		res.end();
	} else if (req.url === '/lights_off') {
		var options = {
		  url: 'http://' + serviceRootUrl + '/actuators/relay', 
		  headers: {"content-type": "application/json",},
		  json: true,
		  method: 'PUT',
		  body: 'I am an attachment',
		  json: {
			  "id": 1,
			  "gpio": 2,
			  "status": 1,
		  }
		};
		request(options, function (err, res, body) {
			if (err) throw (err);
			if (res && res.statusCode === 200) {
				console.log('Json przesłany do serwera!');
			}
		});
		res.writeHeader(200, {"Content-Type": "text/plain"});
		res.write('Json przesłany do serwera!');
		res.end();
  } else {
    res.writeHeader(404, {"Content-Type": "text/plain"});
    res.write('Proszę użyć właściwej ścieżki. - 404');
    res.end();
  }

}).listen(portServer);

wsClient = new WebSocket ('ws://' + serviceRootUrl + ':' + portWebSocketClient);
wsServer = new WebSocket.Server ({server});
var DHTdata;
var BUTTONdata; 

function broadcast(DHT) {
	wsServer.clients.forEach(function each(client) {
		if (client.readyState === WebSocket.OPEN) {
		  client.send(DHT);
		}
	});
};	

wsServer.on('connection', function connection(Socket) {
	broadcast(BUTTONdata);
	Socket.on('message', function incoming(data) {
		console.log(data);
		wsServer.clients.forEach(function each(client) {
		  if (client.readyState === WebSocket.OPEN) {
			BUTTONdata = data;
			client.send(data);
		  }
		});
    });
});

wsClient.on('message', function incoming(data) {
	if (DHTdata != data) {
		DHTdata = data;
		broadcast(DHTdata);
	}	
});

console.log('Serwer działa na adresie http://localhost:' + portServer);