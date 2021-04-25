
var I2S_SAMPLE_RATE = 16000;
var I2S_SAMPLE_BITS = 16;
var RECORDINGS_TIME = 5;
var I2S_CHANNEL_NUM = 1;

var header = [0x52, 0x49, 0x46, 0x46, 0x24, 0xE2, 0x04, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 
			  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x00, 0x7D, 0x00, 0x00, 
			  0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x00, 0xE2, 0x04, 0x00];

var Received_Packet_Counter = 0;
var dataSave;
var flash_wr_size = 0;
var FLASH_RECORD_SIZE = 0;
new function() {
	var ws = null;
	var connected = false;

	var serverUrl;
	var connectionStatus;
	var sendMessage;
	
	var connectButton;
	var disconnectButton; 
	var sendButton;

	var waveHeader = function(){

		FLASH_RECORD_SIZE =(I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORDINGS_TIME);

		var fileSize = FLASH_RECORD_SIZE + 36;

		header[4] = (fileSize & 0xFF);
		header[5] = ((fileSize >> 8) & 0xFF);
		header[6] = ((fileSize >> 16) & 0xFF);
		header[7] = ((fileSize >> 24) & 0xFF);

		///********** Type of Format  **********///
		header[20] = 0x01;
		header[21] = 0x00;

		///**********   Num Channels  **********///
		header[22] = (I2S_CHANNEL_NUM & 0xFF);
        header[23] = ((I2S_CHANNEL_NUM >> 8) & 0xFF);

		///**********   Smaple Rate   **********///
		header[24] = (I2S_SAMPLE_RATE & 0xFF);
		header[25] = ((I2S_SAMPLE_RATE >> 8) & 0xFF);
		header[26] = ((I2S_SAMPLE_RATE >> 16) & 0xFF);
		header[27] = ((I2S_SAMPLE_RATE >> 24) & 0xFF);

		///**********    Byte Rate    **********///
		var byteRate = (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8);
		header[28] = (byteRate & 0xFF);
		header[29] = ((byteRate >> 8) & 0xFF);
		header[30] = ((byteRate >> 16) & 0xFF);
		header[31] = ((byteRate >> 24) & 0xFF);

		///**********   Block Align   **********///
		var blockAlign = (I2S_SAMPLE_BITS * I2S_CHANNEL_NUM / 8);
		header[32] = (blockAlign & 0xFF);
  		header[33] = ((blockAlign >> 8) & 0xFF);	

		///********** Bits per sample **********///
	    header[34] = (I2S_SAMPLE_BITS & 0xFF);
	    header[35] = ((I2S_SAMPLE_BITS >> 8) & 0xFF);

	    ///********** Bits per sample **********///
		header[40] = (FLASH_RECORD_SIZE & 0xFF);
		header[41] = ((FLASH_RECORD_SIZE >> 8) & 0xFF);
		header[42] = ((FLASH_RECORD_SIZE >> 16) & 0xFF);
		header[43] = ((FLASH_RECORD_SIZE >> 24) & 0xFF);

		$('#messages').html('');
		for (var i = 0; i < header.length; i++) {
		  // $("#messages").append(String.fromCharCode(header[i]));
		  if(i === 0){
		  	dataSave = String.fromCharCode(header[i]);
		  } else {
		  	dataSave += String.fromCharCode(header[i]);
		  }
		}
	}

	var open = function() {
		var url = serverUrl.val();
		ws = new WebSocket(url);
		ws.binaryType = 'arraybuffer';
		ws.onopen = onOpen;
		ws.onclose = onClose;
		ws.onmessage = onMessage;
		ws.onerror = onError;

		connectionStatus.text('OPENING ...');
		serverUrl.attr('disabled', 'disabled');
		connectButton.hide();
		disconnectButton.show();
	}
	
	var close = function() {
		if (ws) {
			console.log('CLOSING ...');
			ws.close();
		}
		connected = false;
		connectionStatus.text('CLOSED');

		serverUrl.removeAttr('disabled');
		connectButton.show();
		disconnectButton.hide();
		sendMessage.attr('disabled', 'disabled');
		sendButton.attr('disabled', 'disabled');
	}
	
	var clearLog = function() {
		$('#messages').html('');
	}
	
	var onOpen = function() {
		console.log('OPENED: ' + serverUrl.val());
		connected = true;
		connectionStatus.text('OPENED');
		sendMessage.removeAttr('disabled');
		sendButton.removeAttr('disabled');
	};
	
	var onClose = function() {
		console.log('CLOSED: ' + serverUrl.val());
		ws = null;
	};
	
	var onMessage = function(event) {
		var data;
		if(event.data instanceof ArrayBuffer ){
			data = new Uint8Array(event.data);
		} else if(typeof event.data === "string"){
			data = event.data.split(",").map(Number);
			// console.log(data);
		}

		// console.log(Received_Packet_Counter++);
		// console.log(data);	

		var i;
		for (i = 0; i < data.length; i++) {
		  dataSave +=String.fromCharCode(data[i]);
		}

		var elem = document.getElementById("myBar");
		flash_wr_size = flash_wr_size + data.length;
		if(flash_wr_size > FLASH_RECORD_SIZE && flash_wr_size > 10)
		{
			var Difference = flash_wr_size - FLASH_RECORD_SIZE;
			flash_wr_size = flash_wr_size - Difference;
			document.getElementById("download").click();
			$('#messages').append("*************** Recording Done ***************\n");
			$('#messages').append("*************** Downloading Data***************\n");

		}

		elem.style.width = (flash_wr_size * 100 / FLASH_RECORD_SIZE) + "%";
	};
	
	var onError = function(event) {
		alert(event.data);
	}
	
	var addMessage = function(data, type) {
		var msg = $('<pre>').text(data);
		if (type === 'SENT') {
			msg.addClass('sent');
		}
		var messages = $('#messages');
		messages.append(msg);
		var msgBox = messages.get(0);
		while (msgBox.childNodes.length > 1000) {
			msgBox.removeChild(msgBox.firstChild);
		}
		msgBox.scrollTop = msgBox.scrollHeight;
	}

	WebSocketClient = {
		init: function() {
			serverUrl = $('#serverUrl');
			connectionStatus = $('#connectionStatus');
			sendMessage = $('#sendMessage');
			
			connectButton = $('#connectButton');
			disconnectButton = $('#disconnectButton'); 
			sendButton = $('#sendButton');
			
			connectButton.click(function(e) {
				close();
				open();
			});
		
			disconnectButton.click(function(e) {
				close();
			});
			
			sendButton.click(function(e) {
				var COMMANDS_RECORD = 1;
				var jsonSend = "{\"COMMANDS_RECORD\":"+COMMANDS_RECORD+ 
							   ",\"I2S_SAMPLE_RATE\":"+I2S_SAMPLE_RATE+ 
							   ",\"I2S_SAMPLE_BITS\":"+I2S_SAMPLE_BITS+
							   ",\"RECORDINGS_TIME\":"+RECORDINGS_TIME+
							   ",\"I2S_CHANNEL_NUM\":"+I2S_CHANNEL_NUM+
							   ",\"I2S_PORT_NUMBER\":"+$('#i2sPort').val()+
							   "}";
				waveHeader();
				ws.send(jsonSend);
				Received_Packet_Counter = 0;
				flash_wr_size = 0;
				$('#messages').append("*************** Recording Start ***************");
			});
			
			$('#clearMessage').click(function(e) {
				clearLog();
			});

			$('#download').click(function(e){
				// var dataSave = $("#messages").text();
				// console.log(dataSave);
				var uint8 = new Uint8Array(dataSave.length);
			    for (var i = 0; i <  uint8.length; i++){
			        uint8[i] = dataSave.charCodeAt(i);
			    }

				var textFileAsBlob = new Blob([uint8]);
				var fileNameToSaveAs =  $("#fileName").val(); 
				var downloadLink = document.createElement("a");
				downloadLink.download = fileNameToSaveAs;
				downloadLink.innerHTML = "Download File";
				if (window.webkitURL != null)
				{
					// Chrome allows the link to be clicked
					// without actually adding it to the DOM.
					downloadLink.href = window.webkitURL.createObjectURL(textFileAsBlob);
				}
				else
				{
					// Firefox requires the link to be added to the DOM
					// before it can be clicked.
					downloadLink.href = window.URL.createObjectURL(textFileAsBlob);
					downloadLink.onclick = destroyClickedElement;
					downloadLink.style.display = "none";
					document.body.appendChild(downloadLink);
				}

				downloadLink.click();
			})

			$('#recTime').change(function(e){
				RECORDINGS_TIME = parseInt($('#recTime').val());
			})

			$('#bps').change(function(e){
				I2S_SAMPLE_BITS = parseInt($('#bps').val());
			})

			$('#sampleRate').change(function(e){
				I2S_SAMPLE_RATE = parseInt($('#sampleRate').val());
			})

			$('#channel').change(function(e){
				I2S_CHANNEL_NUM = parseInt($('#channel').val());
			})
			
			var isCtrl;
			sendMessage.keyup(function (e) {
				if(e.which == 17) isCtrl=false;
			}).keydown(function (e) {
				if(e.which == 17) isCtrl=true;
				if(e.which == 13 && isCtrl == true) {
					sendButton.click();
					return false;
				}
			});
		}
	};
}

$(function() {
	WebSocketClient.init();
});
