
/***MetaData***

{
	"name": "ƒ‚ƒfƒ‹ƒŠƒXƒg",
	"icon": "icon.png",
	"api_version": 1
}

***MetaData***/

var list;
list = new UI.List({
	docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT | UI.DOCKING_BOTTOM
});
Model.onReload = function() {
	list.clearItems();
	var even_line = false;
	var model_names = Model.all();
	for (var i = 0; i < model_names.length; i++) {
		if ((/char:/).test(model_names[i])) {
			(function(model_name){
				list.addItem(
					new UI.Label({
						docking: UI.DOCKING_TOP | UI.DOCKING_LEFT | UI.DOCKING_RIGHT,
						text: model_name.substring(5),
						bgcolor: ((even_line = !even_line) ? "#EEAFEECC" : "#FFFFFFCC"),
						onclick: function() {
							Player.stopMotion();
							Account.updateModelName(model_name)
						}
					})
				);
			})(model_names[i])
		}
	}
}

Model.onReload();

Card.board.width = 240;
Card.board.height = 120;
Card.board.docking = UI.DOCKING_BOTTOM | UI.DOCKING_RIGHT
Card.board.addChild(list);