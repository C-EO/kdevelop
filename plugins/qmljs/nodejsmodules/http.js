/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var event = require("event");

var net = require("net");

exports.STATUS_CODES = {};

exports.createServer = function (requestListener) { return new Server(); };
exports.createServer(function(){});

exports.createClient = function (port, host) { return new Client(); };
exports.createClient(1, "");

/*
 * exports.Server
 */
function Server() { return ; }
exports.Server = Server;
exports.Server.prototype = event.EventEmitter;

exports.Server.prototype.listen = function (port, hostname, backlog, callback) { return ; };
exports.Server.prototype.listen(1, "", 1, function(){});

exports.Server.prototype.close = function (callback) { return ; };
exports.Server.prototype.close(function(){});

exports.Server.prototype.maxHeadersCount = 1;

exports.Server.prototype.setTimeout = function (msecs, callback) { return ; };
exports.Server.prototype.setTimeout(1, function(){});

exports.Server.prototype.timeout = 1;


/*
 * exports.ServerResponse
 */
function ServerResponse() { return ; }
exports.ServerResponse = ServerResponse;
exports.ServerResponse.prototype = event.EventEmitter;

exports.ServerResponse.prototype.writeContinue = function () { return ; };
exports.ServerResponse.prototype.writeContinue();

exports.ServerResponse.prototype.writeHead = function (statusCode, reasonPhrase, headers) { return ; };
exports.ServerResponse.prototype.writeHead(1, "", new Object());

exports.ServerResponse.prototype.setTimeout = function (msecs, callback) { return ; };
exports.ServerResponse.prototype.setTimeout(1, function(){});

exports.ServerResponse.prototype.statusCode = 1;

exports.ServerResponse.prototype.setHeader = function (name, value) { return ; };
exports.ServerResponse.prototype.setHeader("", "");

exports.ServerResponse.prototype.headersSent = true;

exports.ServerResponse.prototype.sendDate = true;

exports.ServerResponse.prototype.getHeader = function (name) { return ""; };
exports.ServerResponse.prototype.getHeader("");

exports.ServerResponse.prototype.removeHeader = function (name) { return ; };
exports.ServerResponse.prototype.removeHeader("");

exports.ServerResponse.prototype.write = function (chunk, encoding) { return true; };
exports.ServerResponse.prototype.write("", "");

exports.ServerResponse.prototype.addTrailers = function (headers) { return ; };
exports.ServerResponse.prototype.addTrailers(new Object());

exports.ServerResponse.prototype.end = function (data, encoding) { return ; };
exports.ServerResponse.prototype.end("", "");


exports.request = function (options, callback) { return new ClientRequest(); };
exports.request(new Object(), function(){});

exports.get = function (options, callback) { return new ClientRequest(); };
exports.get(new Object(), function(){});

/*
 * exports.Agent
 */
function Agent() { return ; }
exports.Agent = Agent;

exports.Agent.prototype.maxSockets = 1;

exports.Agent.prototype.sockets = [];

exports.Agent.prototype.requests = [];


exports.globalAgent = new Agent();

/*
 * exports.ClientRequest
 */
function ClientRequest() { return ; }
exports.ClientRequest = ClientRequest;
exports.ClientRequest.prototype = event.EventEmitter;

exports.ClientRequest.prototype.write = function (chunk, encoding) { return ; };
exports.ClientRequest.prototype.write("", "");

exports.ClientRequest.prototype.end = function (data, encoding) { return ; };
exports.ClientRequest.prototype.end("", "");

exports.ClientRequest.prototype.abort = function () { return ; };
exports.ClientRequest.prototype.abort();

exports.ClientRequest.prototype.setTimeout = function (msecs, callback) { return ; };
exports.ClientRequest.prototype.setTimeout(1, function(){});

exports.ClientRequest.prototype.setNoDelay = function (noDelay) { return ; };
exports.ClientRequest.prototype.setNoDelay(true);

exports.ClientRequest.prototype.setSocketKeepAlive = function (enable, initialDelay) { return ; };
exports.ClientRequest.prototype.setSocketKeepAlive(true, 1);


/*
 * exports.IncomingMessage
 */
function IncomingMessage() { return ; }
exports.IncomingMessage = IncomingMessage;
exports.IncomingMessage.prototype = event.EventEmitter;

exports.IncomingMessage.prototype.httpVersion = "";

exports.IncomingMessage.prototype.headers = new Object();

exports.IncomingMessage.prototype.trailers = new Object();

exports.IncomingMessage.prototype.setTimeout = function (msecs, callback) { return ; };
exports.IncomingMessage.prototype.setTimeout(1, function(){});

exports.IncomingMessage.prototype.method = "";

exports.IncomingMessage.prototype.url = "";

exports.IncomingMessage.prototype.statusCode = 1;

exports.IncomingMessage.prototype.socket = new net.Socket();


