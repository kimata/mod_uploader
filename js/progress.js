var uploadID = 0;
var startTime;
var progressInfo;
var progressBar;
var originalAction = '';
var updateTimer = null;
var checkTimer = null;
var failCount;
var state;
var firstRequest = true;
var infoColor = null;

var PROGRESS_BAR_WIDTH = 600;
var STATE_START = 0;
var STATE_ERROR = 1;

function startUpload()
{
    var errorMessage;

    // ID is more than 1.
    uploadID    = Math.floor(Math.random()*(Math.pow(2, 32)-1)) + 1;
    startTime   = getTime();
    progressInfo= document.getElementById('progress_info');
    progressBar = document.getElementById('progress_bar');

    if (originalAction == '') {
        originalAction = this.action;
    }
    this.action = originalAction + '/' + uploadID;    

    if (infoColor == null) {
        infoColor = progressInfo.style.color;
    } else {
        progressInfo.style.color = infoColor;
    }
    if (!firstRequest) {
        errorMessage = getResponseDocument().getElementById('error_message');
        if (errorMessage != null) {
            errorMessage.innerHTML = '';
        }
    }
    firstRequest = false;

    progressInfo.innerHTML = 'Loading...';
    document.getElementById('progress').style.display = 'block';

    clearTimer();
    failCount = 0;
    state = STATE_START;

    startUpdateProgress();

    return true;
}

function startUpdateProgress()
{
    // reload every 1 sec.
    updateTimer = setInterval('updateProgress()', 1000);
    checkTimer = setInterval('checkResponse()', 2000);
}

function clearTimer()
{
    if (updateTimer != null) {
        clearInterval(updateTimer);
        updateTimer = null;
    }
    if (checkTimer != null) {
        clearInterval(checkTimer);
        checkTimer = null;
    }
}

function setProgresssInfo(text)
{
    if (state != STATE_ERROR) {
        progressInfo.innerHTML = text;
    }
}

function updateProgress()
{
    var progress;

    progress = getProgress(uploaderURL + '/progress_data/' + uploadID);

    if (progress == null) {
        if (++failCount > 10) {
            clearTimer();
        }
        setProgresssInfo('Server Busy.');
        return;
    } else {
        failCount = 0;
    }

    progressBar.width = PROGRESS_BAR_WIDTH * progress.getPercent() / 100;

    if (progress.isFinish) {
        clearTimer();
        setProgresssInfo('Upload Succeeded!');
        
        setInterval('document.location = "' + uploaderURL + '/info/upload_id/' + uploadID + '"',
                    1000);

        return;
    }

    setProgresssInfo(getProgressMessage(progress));
}

function getResponseDocument()
{
    if (document.getElementById('dummy').contentDocument) {
        return document.getElementById('dummy').contentDocument;
    } else {
        return window.dummy.document;
    }
}

function checkResponse()
{
    var dummyFrame;
    var responseDocument;
    var errorMessage;
 
    responseDocument = getResponseDocument();

    try {
        errorMessage = responseDocument.getElementById('error_message');

        if ((errorMessage == null) || (errorMessage.innerHTML == '')) {
            return;
        }
    } catch(e) {
        return;
    }
    
    clearTimer();
    
    state = STATE_ERROR;
    progressInfo.style.color = 'red';
    progressInfo.innerHTML = 'Error: ' + errorMessage.innerHTML;
    progressBar.width = PROGRESS_BAR_WIDTH;
}

function getTime()
{
    return (new Date()).getTime() / 1000;
}

function getProgressMessage(progress)
{
    if (progress.totalSize == 0) {
        return progress.readSize + ' / Unkown size';
    } else {
        return progress.readSize + ' / ' + progress.totalSize +
            ' (' + progress.getPercent() + '%) @ ' + progress.getSpeed() +
            ' [' + progress.getRemainTime() + ']';
    }
}

function getProgress(url)
{
    try {
        var request;
        var response;
        var timer;

        request = createHTTPRequest();
        if (request.readyState != 4) {
            request.abort();
        }

        timer = setTimeout(function () {
            request.abort();
            clearTimeout(timer);
        }, 3000);

        request.open('POST', url, false);
        request.send("GET progress");
        clearTimeout(timer);

        if ((request.status != 200) ||
            ((request.responseText.charAt(0) != 's') &&
             (request.responseText.charAt(0) != 'S'))) {
            return null;
        }
        response = request.responseText.split(' ');

        return new Progress(response[0], response[1], response[2]);
    } catch(e) {
        return null;  
    }
}

function createHTTPRequest()
{
    var request;

    try {
        request = new ActiveXObject('Msxml2.XMLHTTP');
    } catch(e) {
        try {
            request = new ActiveXObject('Microsoft.XMLHTTP');
        } catch (e) {
            request = false;
        }
    }

    if (!request) {
        if (typeof XMLHttpRequest != 'undefined') {
            request = new XMLHttpRequest();
        }
    }

    return request;
}

function normalizeNumber(number)
{
    if (number*10%10 == 0) {
        number += '.0';
    }
    return number;
}

function Progress(stat, totalSize, readSize)
{
    this.isFinish   = (stat == 'S');
    this.totalSize  = totalSize;
    this.readSize   = readSize;
    this.getPercent	= function()
    {
        var percent;
         percent = Math.round(readSize*100*10/totalSize) / 10;
         return normalizeNumber(percent);
    }
    this.getSpeed = function()
    {
        var elapsedTime;
        var bitPerSec;
        elapsedTime = getTime() - startTime;
        if (elapsedTime < 1) {
            return '--------';
        }
        bitPerSec = readSize*8 / elapsedTime;
        if (bitPerSec > 1024*1024*1024) {
            return normalizeNumber(Math.round(bitPerSec*10/(1024*1024*1024))/10) + ' Gbps';
        } else if (bitPerSec > 1024*1024) {
            return normalizeNumber(Math.round(bitPerSec*10/(1024*1024))/10) + ' Mbps';
        } else if (bitPerSec > 1024) {
            return normalizeNumber(Math.round(bitPerSec*10/1024)/10) + ' Kbps';
        } else {
            return normalizeNumber(Math.round(bitPerSec*10)/10) + ' bps';
        }
    }
    this.getRemainTime = function()
    {
        var elapsedTime;
        var bytePerSec;
        var remainSec;
        var hour;
        var min;
        var sec;
        elapsedTime = getTime() - startTime;
        if (elapsedTime < 1) {
            return '--:--:--';
        }
        bytePerSec = readSize / elapsedTime;
        remainSec = (totalSize-readSize) / bytePerSec;
        hour = Math.floor(remainSec/3600);
        if (hour < 10) {
            hour = '0' + hour;
        }
        min = Math.floor((remainSec%3600)/60);
        if (min < 10) {
            min = '0' + min;
        }
        sec = Math.floor(remainSec%60);
        if (sec < 10) {
            sec = '0' + sec;
        }
        return hour + ':' + min + ':' + sec;
    }
    return this;
}

// Local Variables:
// mode: javascript-generic
// End:
