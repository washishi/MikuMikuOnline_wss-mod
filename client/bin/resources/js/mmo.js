(function(){
//	"use strict";

//
// MMO Client Function Script
//
    /**
    * @module global
    */

	// Global Functions
	global.print = 		function(text){ return Script.print(text); }
	global.info = 			function(){ return Script.info(); }
	global.setTimeout = 	function(func, time){ return Script.setTimeout(func, time); }
	global.setInterval = 	function(func, time){ return Script.setInterval(func, time); }
	global.clearTimeout = 	function(id){ return Script.clearTimeout(id); }
	global.clearInterval = 	function(id){ return Script.clearInterval(id); }	
	
	// LocalStorage
	/*
	global.localStorage.setItem = function(key, value){ Card.localStorage[key] = value; }
	global.localStorage.getItem = function(key){ return Card.localStorage[key]; }
	// Card.global.localStorage = 	Card.localStorage;
	*/
	
	// Commands
	
	global.Commands = {};
	global.parseCommand = 
	function(text) {
		var parsed_text = text.match(/^\/(\w{1,8})\s?(.*)$/)
		if (parsed_text) {
			var command = parsed_text[1];
			var args = parsed_text[2].trim().split(" ");
			var func = global.Commands[command];
			if (Object.isFunction(func)) {
				return func(args[0], args[1], args[2], args[3], args[4], args[5]);
			}
		}
	};
	
	// Network
	/**
    * @class Network
    * @static
    */
     
    /**
     * サーバーからメッセージが送られてきた時に呼ばれます
     *
     * メッセージ情報から、メッセージが送信された時間や送信者を取得できます
     *
     * @event onReceive
     * @param {Object} info メッセージ情報
     * @param {Object} msg メッセージオブジェクト
     * @static
     */
	Network._onReceiveJSON = function(info_json, msg_json) {
		if (typeof Network.onReceive === 'function') {
		
			var info = JSON.parse(info_json)
			var msg = JSON.parse(msg_json)
			
			// 時間をDateオブジェクトに変換
			if (info.time) {
				info.time = Date.create(info.time);
			}
			
			if (info.id) {
				info.player = Player.getFromId(info.id)
			}
			
			Network.onReceive(info, msg)
		}
	}

    /**
     * メッセージを送信します
     *
     * @method sendAll
     * @param {Object} msg メッセージオブジェクト
     *
     * @static
     */
	Network.sendAll = function(msgObject) {
		Network._sendJSONAll(JSON.stringify(msgObject));
	}
	
	
	// UI Constructors
	
	UI.Board = function(options) {
		options = options || {};
		var defaults = {
			height: 200,
			width: 200,
			docking: UI.DOCKING_TOP | UI.DOCKING_LEFT
		};
		Object.merge(options, defaults, false, false);
		return Object.merge(new UI._Board, options);
	};
	
	UI.Button = function(options) {
		options = options || {};
		var defaults = {
			height: 32,
			docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT
		};
		Object.merge(options, defaults, false, false);
		return Object.merge(new UI._Button, options);
	};
	
	UI.Label = function(options) {
		options = options || {};
		var defaults = {
			height: 32,
			docking: UI.DOCKING_TOP | UI.DOCKING_LEFT,
			color: "#000000FF",
			bgcolor: "#00000000"
		};
		Object.merge(options, defaults, false, false);
		return Object.merge(new UI._Label, options);
	};
	
	UI.List = function(options) {
		options = options || {};
		var defaults = {
			height: 120,
			docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT
		};
		Object.merge(options, defaults, false, false);
		return Object.merge(new UI._List, options);
	};
	
	UI.Group = function(options) {
		options = options || {};
		var defaults = {
			docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT
		};
		Object.merge(options, defaults, false, false);
		return Object.merge(new UI._Group, options);
	};

	// PlayerFunctions

	Player.myself = function(){
	        return Player.all().find(function (p) {
	            return p.id() == Account.id();
	        });
	};
	
	Player.getFromName = function(name){
	        return Player.all().find(function (p) {
	            return p.name() == name;
	        });
	};

})();