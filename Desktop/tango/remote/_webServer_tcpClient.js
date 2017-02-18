var webServer = require('http').createServer(handler);
var io = require('socket.io')(webServer);
var fs = require('fs');
var url = require('url');

var port = 8088;
var formatTag = "text/html";
var filePath = "";
var msgType = 0;
var showDir = 0;

// 641.5.1007 hiroshi: TCP client part
var HOST = '127.0.0.1';
var TCP_PORT = 6969;

var net = require('net');
var client = new net.Socket();

// Add a 'data' event handler for the client socket
// data is what the server sent to this socket
client.on('data', function(data) {
    
    console.log('DATA: ' + data);
    // Close the client socket completely
//    client.destroy();
});

// Add a 'close' event handler for the client socket
client.on('close', function() {
    console.log('Connection closed');
});

client.connect(TCP_PORT, HOST, function() {
	console.log('CONNECTED TO: ' + HOST + ':' + TCP_PORT);
});



// for authentication procedure state

webServer.listen(port);

// http server handler
function handler(request, response) {  
    
    // Parse the request containing file name
    var fileName = url.parse(request.url).pathname;
    // Parse the request whole message
    var msg = request.url;
    console.log(msg);
    console.log(fileName);
	
    // show remote client's IP address
    //console.log(request.headers['x-forwarded-for']);
    //console.log(request.connection.remoteAddress);
    //console.log(request.socket.remoteAddress);
////	console.log(request.connection.socket.remoteAddress);
	
    // authentication control
    if (msg.endsWith('c.html') || msg.endsWith('a.html') || msg.endsWith('moniz.jpg')) {
	filePath = "";
	formatTag = "text/html";
	msgType = 0; // normal
	fileName = "/b.html";
    }
    // handles various request from client
    else if (msg.endsWith(".html")) {
	filePath = "";
	formatTag = "text/html";
	msgType = 0; // normal
    } else if (msg.endsWith(".ico")) {
	filePath = "/icons/";
	formatTag = "icon/ico";
	msgType = 1; // image
    } else if (msg.endsWith(".jpg")) {
	filePath = "/images/";
	formatTag = "image/jpeg";
	msgType = 1; // image
    } else if (msg.endsWith(".gif")) {
	filePath = "/images/";
	formatTag = "image/gif";
	msgType = 1; // image
    } else if (msg.endsWith(".png")) {
	filePath = "/images/";
	formatTag = "image/png";
	msgType = 1; // image
    } 
    // server-sent events
    else if (msg.endsWith("srvSent")) {
	filePath = "";
	formatTag = "srvSent";
	msgType = 3; // srvSent
    }
	// authentication handler
    else if (msg.indexOf("?username") > -1)
    {
	if (msg.endsWith("?username=jack&password=hiroshi")) {
	    if (msg.startsWith("/kelia.jpg"))
	    {
		msgType = 1;//image
		filePath = "/images/";
		formatTag = "image/jpeg";
		fileName = "/kelia.jpg";
	    }
	    else if (msg.startsWith("/moniz.jpg"))
	    {
		msgType = 1;//image
		filePath = "/images/";
		formatTag = "image/jpeg";
		fileName = "/moniz.jpg";
		client.destroy();
	    }
	    else if (msg.startsWith("/c.html"))
	    {
		msgType = 0;
		filePath = "";
		formatTag = "text/html";
		fileName = "/c.html";
	    }
	    else
	    {
		msgType = 0;
		filePath = "";
		formatTag = "text/html";
		fileName = "/a.html";
	    }
	}
	else {
	    filePath = "";
	    formatTag = "text/html";
	    msgType = 0; // normal
	    fileName = "/b.html";
	}
    
	console.log("!!")
    }
    else
    {
	console.log("no....");
	msgType = -1;
    }

    pathname = filePath + fileName;
   
    // Print the name of the file for which request is made.
    console.log("Request for " + pathname + " received.");
    console.log("msgType: " + msgType);

    // 'GET/POST' response to client
    if (msgType == 3)
    {
	response.writeHead(200, {
	    'Content-Type': 'text/event-stream',
	    'Cache-Control': 'no-cache',
	    'Connection': 'keep-alive'
	});	
		
	setInterval(function() {
	    var labels = ['message', 'cmdUp'];
	    var srvData = [new Date().getTime(), showDir];
	    for (var i = 0; i < labels.length; i++)
	    {
		response.write('event: ' + labels[i] + '\n');
		response.write('data: ' + srvData[i] + '\n\n');
	    }
	}, 1000);
//		response.end(); // if setInterval used, no need to response.end()! 
    }   
    else if (msgType == 0 || msgType == 1)
    {
	// Read the requested file content from file system
	console.log(pathname.substr(1));
	fs.readFile(pathname.substr(1), function (err, data) {
	    if (err) 
	    {
		console.log(err);
		// HTTP Status: 404 : NOT FOUND
		// Content Type: text/plain
		response.writeHead(404, {'Content-Type': 'text/html'});
	    }
	    else
	    {		
		if (msgType == 0)
		{
		    //html found	  
		    // HTTP Status: 200 : OK
		    // Content Type: text/html
		    response.writeHead(200, {'Content-Type': formatTag});	
         
		    // Write the content of the file to response body
		    response.write(data.toString());
		}
		else if (msgType == 1)
		{
		    //image found	  
		    // HTTP Status: 200 : OK
		    // Content Type: image/*
		    response.writeHead(200, {'Content-Type': formatTag});	         
		    // Write the content of the file to response body
		    response.write(data);
		}
	    }
	    // Send the response body 
	    response.end();
	}); 
    }   
}

// Console will print the message
console.log('Server running at http://127.0.0.1:',port);

io.on('connection', function(socket) {
    // add code here
    socket.emit('foo', {name: 'bar'}); // send
    socket.on('baz', function(data) {
	console.log(data); // receive
    });
    // receive dir cmd
    socket.on('cmd-dir', function(data) {
	console.log(data.cmd);
	socket.emit('cmd-dir-confirmed', data);
	// Write a message to the socket as soon as the client is connected, the server will receive it as message from the client 
	client.write(data.cmd);
    });
});