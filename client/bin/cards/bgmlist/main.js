
/***MetaData***

{
	"name": "BGMƒŠƒXƒg",
	"icon": "icon.png",
	"api_version": 1
}

***MetaData***/

var list;
list = new UI.List({
	docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT | UI.DOCKING_BOTTOM
});

var even_line = false;
var music_names = Music.all();
for (var i = 0; i < music_names.length; i++) {
    (function (music_name) {
        list.addItem(
				new UI.Label({
				    docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT,
				    text: music_name,
				    bgcolor: ((even_line = !even_line) ? "#EEAFEECC" : "#FFFFFFCC"),
				    onclick: function () {
				        Music.play(music_name,true)
				    }
				})
			);
    })(music_names[i])
}

Card.board.width = 240;
Card.board.height = 120;
Card.board.docking = UI.DOCKING_BOTTOM | UI.DOCKING_LEFT
Card.board.addChild(list);