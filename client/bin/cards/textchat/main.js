
/***MetaData***

{
	"name": "チャット",
	"icon": "icon.png",
	"api_version": 1
}

***MetaData***/

// 吹き出しの表示時間（ミリ秒）
var BALLOON_EXPIRATION = 40000

var list;

// チャットメッセージ受信
var even_line = false;
Network.onReceive = function (info, msg) {
    if (msg.body && !msg.private) {
        list.addItem(
		new UI.Label({
		    docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT,
		    text: "[" + info.player.name() + "] " + msg.body,
		    bgcolor: ((even_line = !even_line) ? "#AFEEEECC" : "#FFFFFFCC")
		})
	    );
    }
    if (msg.private) {
        if (Account.name() == msg.private || info.player.name() == Account.name()) {
            list.addItem(
		new UI.Label({
		    docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT,
		    text: "[private:" + info.player.name() + "] " + msg.body,
		    bgcolor: ((even_line = !even_line) ? "#add8e6CC" : "#87ceebCC")
		})
	    );
        }
    }
    if (msg.system) {
        list.addItem(
		new UI.Label({
		    docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT,
		    text: "[サーバーより] " + msg.system,
		    bgcolor: "#FFFACDCC"
		})
	    );
    }
    list.scroll_y = 999999;

    if (info.player && msg.body && !msg.private) {

        // 吹き出しを表示
        info.player.setBalloonContent(
			new UI.Label({
			    width: Screen.width() / 6,
			    text: msg.body
			})
		);

        // 一定時間経過後に吹き出しを消す
        clearTimeout(info.player.baloon_timer)
        info.player.baloon_timer = setTimeout(function () {
            info.player.setBalloonContent(null)
        }, BALLOON_EXPIRATION);
    }

}

Player.onLogin = function(player) {
	list.addItem(
		new UI.Label({
			docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT,
			text: player.name() + "さんがログインしました",
			bgcolor: "#FFCC66CC",
			width: 0
		})
	);
	list.scroll_y = 999999;
}

Player.onLogout = function(player) {
	list.addItem(
		new UI.Label({
			docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT,
			text: player.name() + "さんがログアウトしました",
			bgcolor: "#CCFF66CC",
			width: 0
		})
	);
	list.scroll_y = 999999;
}

// チャットメッセージ送信
InputBox.onEnter = function (text) {

	//サイコロ用
	var dice_parsed_text = text.match(/^\/(\d+)[Dd](\d+)/)
	if (dice_parsed_text) {
		var time = dice_parsed_text[1]
        var size = dice_parsed_text[2]
        var msg = "【ダイス /" + time + "D" + size + "】\n "
        for (var i = 0; i < time; i++) {
        	msg += Number.random(1, size) + ", "
        }
        
        var msgObject = { body: msg };
        Network.sendAll(msgObject);
		return;
	}

    // コマンドを解析
    var parsed_text = text.match(/^\/(\w{1,8})\s?(\S*)/)
    if (parsed_text) {

        var command = parsed_text[1]
        var args = parsed_text[2]

        switch (command) {

            // ニックネームを変更       
            case "nick":
                Account.updateName(args.trim());
                break;

            // モデルを変更       
            case "model":
                Account.updateModelName("char:" + args.trim());
                break;

            // プレイヤー位置をリセット       
            case "escape":
                Player.escape();
                break;
                
            // リロード
            case "reload":
                Model.rebuild();
                break;
                
            // システム
            case "system":
                var msgObject = { system: args.trim() };
                Network.sendAll(msgObject);
                break;
                
            // プライベート
            case "private":
                args.trim();
                var tok = args.split(" ");
                var msgObject = {
                    prvate: tok[0],
                    body: tok[1]
                    };
                Network.sendAll(msgObject);
                break;
        }

    } else {
        // コマンドでない場合はそのままチャットメッセージとして送信
        var msgObject = { body: text };
        Network.sendAll(msgObject);
    }

};

list = new UI.List({
	docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT | UI.DOCKING_BOTTOM
});

Card.board.width = 380;
Card.board.addChild(list);