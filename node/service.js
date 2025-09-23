"use strict"

const http = require('node:http')
const path = require('node:path')
const fs = require('node:fs')

const flushingPeriod = 30 * 60 * 1000 // 30 min
const listeningPort = 8080

// Example web service to gather data from Meteo Logger
// AHT20: temperature, humidity
// BMP280: pressure, temperture
// Launch: cls && node service.js
// http://localhost:8080/push-data?stream=meteo&data=1757423835;28.6;75.2;840.5;28.5
// http://localhost:8080/flush-streams

const timeFormatter = new Intl.DateTimeFormat("en-US",
    { hour: "numeric", minute: "numeric", second: "numeric", hour12: false })

const streamFiles = {
    'meteo': '../.streams/meteo.csv'
}

const streamCache = {
    'meteo': []
}

/** @param {string} msg @param {http.IncomingMessage | undefined} req @return {string} */
function writeLog(msg, req) {
    if (req) console.log(timeFormatter.format(new Date()),
        req.socket.remoteAddress + ':' + req.socket.remotePort, req.url, ' --> ', msg)
    else console.log(timeFormatter.format(new Date()), msg)
    return msg
}

/** @return {string[]} */
function flushStreams() {
    let failed = []
    for (const sname in streamCache) {
        const file = streamFiles[sname]
        const data = streamCache[sname]
        if (data.length > 0) {
            if (!fs.existsSync(path.dirname(file))) {
                fs.mkdirSync(path.dirname(file));
            }
            fs.appendFile(file, data.join('\n') + '\n', err => {
                if (err)
                    failed.push(sname)
                else data.length = 0
            })
        }
    }
    return failed
}

const server = http.createServer((req, res) => {
    res.shouldKeepAlive = false
    const [url, enc] = decodeURIComponent(req.url).split('?')
    const dta = {}
    if (enc) {
        for (const p of enc.split('&')) {
            const [k, v] = p.split('=')
            dta[k] = v
        }
    }

    if (url === '/push-data' && req.method === 'GET') {
        const file = streamFiles[dta.stream]
        const data = streamCache[dta.stream]
        if (file) {
            data.push(dta.data)
            res.statusCode = 200
            res.end(writeLog('OK', req))
            // res.writeHead(200, "OK", {'Content-Type': 'text/plain'});
            // res.end();
        }
        else res.end(writeLog(`Invalid stream '${dta.stream}'`, req))
    }
    else if (url === '/flush-streams' && req.method === 'GET') {
        const failed = flushStreams()
        if (failed.length > 0) {
            res.end(writeLog(`Flush failed for ${failed.join(', ')}`, req))
        }
        else res.end(writeLog('OK', req))
    }
    else res.end(writeLog(`Invalid url '${url}'`, req))
})

setInterval(() => {
    let cached = 0
    for (const sname in streamCache)
        cached += streamCache[sname].length
    if (cached > 0) {
        const failed = flushStreams()
        const resume = failed.length > 0 ? `failed ${failed.join(', ')}` : 'OK'
        writeLog(`Flushing cached data (${cached})  -->  ${resume}`)
}
}, flushingPeriod)

server.listen(listeningPort)
console.log('Data gathering service started on port', listeningPort)
