-- translation for ThicketPackage

return {
	["thicket"] = "林包",

	["#caopi"] = "霸业的继承者",
	["caopi"] = "曹丕",
	["xingshang"] = "行殇",
	[":xingshang"] = "其他角色死亡时，你可以获得该角色的牌。",
	["fangzhu"] = "放逐",
	[":fangzhu"] = "每当你受到一次伤害后，你可以令一名其他角色摸X张牌，然后将其武将牌翻面。（X为你已损失的体力值）",
	["songwei"] = "颂威",
	[":songwei"] = "<font color=\"orange\"><b>主公技。</b></font>其他魏势力角色的黑色判定牌生效后，该角色可以令你摸一张牌。",
	["songwei:yes"] = "令曹丕摸一张牌",
	["xingshang:yes"] = "你可以获得死亡角色的牌",
	["@fangzhu"] = "请选择“放逐”的目标",
	["~fangzhu"] = "选择一名其他角色→点击确定",

	["#xuhuang"] = "周亚夫之风",
	["xuhuang"] = "徐晃",
	["duanliang"] = "断粮",
	[":duanliang"] = "你可以将一张黑色的基本牌或黑色的装备牌当【兵粮寸断】使用。你可以对距离2以内的其他角色使用【兵粮寸断】。",

	["#menghuo"] = "南蛮王",
	["menghuo"] = "孟获",
	["huoshou"] = "祸首",
	[":huoshou"] = "<font color=\"blue\"><b>锁定技。</b></font>【南蛮入侵】对你无效。你是【南蛮入侵】的伤害来源。",
	["zaiqi"] = "再起",
	[":zaiqi"] = "摸牌阶段开始时，若你已受伤，你可以放弃摸牌并展示牌堆顶的X张牌：每有一张<font color=\"red\">♥</font>牌，你回复1点体力，然后弃置所有<font color=\"red\">♥</font>牌并获得其余的牌。（X为你已损失的体力值）",
	["#HuoshouTransfer"] = "%from 的“%arg2”被触发，【<font color=\"yellow\"><b>南蛮入侵</b></font>】的伤害来源改为 %from",
	["zaiqi:yes"] = "你可以放弃摸牌并展示牌堆顶的X张牌：每有一张<font color=\"red\">♥</font>牌，你回复1点体力，然后弃置所有<font color=\"red\">♥</font>牌并获得其余的牌。（X为你已损失的体力值）",

	["#zhurong"] = "野性的女王",
	["zhurong"] = "祝融",
	["juxiang"] = "巨象",
	[":juxiang"] = "<font color=\"blue\"><b>锁定技。</b></font>【南蛮入侵】对你无效。其他角色使用的【南蛮入侵】在结算完毕后进入弃牌堆时，你获得之。",
	["lieren"] = "烈刃",
	[":lieren"] = "每当你使用【杀】对目标角色造成一次伤害后，你可以与该角色拼点：若你赢，你获得其一张牌。",

	["#sunjian"] = "武烈帝",
	["sunjian"] = "孙坚",
	["yinghun"] = "英魂",
	[":yinghun"] = "回合开始阶段开始时，若你已受伤，你可以选择一名其他角色并选择一项：令其摸一张牌，然后弃置X张牌，或令其摸X张牌，然后弃置一张牌。（X为你已损失的体力值）",
	["@yinghun"] = "请选择“英魂”的目标",
	["~yinghun"] = "选择一名其他角色→点击确定",
	[":yinghun:"] = "请选择一项:",
	["yinghun:d1tx"] = "摸一张牌，然后弃置X张牌",
	["yinghun:dxt1"] = "摸X张牌，然后弃置一张牌",

	["#lusu"] = "独断的外交家",
	["lusu"] = "鲁肃",
	["haoshi"] = "好施",
	[":haoshi"] = "摸牌阶段，你可以额外摸两张牌：若你拥有五张或更多的手牌，你将一半数量（向下取整）的手牌交给除你外场上手牌数最少的一名角色。",
	["dimeng"] = "缔盟",
	[":dimeng"] = "<font color=\"green\"><b>阶段技。</b></font>出牌阶段，你可以选择两名其他角色并弃置等于其手牌数差数量的牌：若如此做，这两名角色交换他们的手牌。",
	["@haoshi"] = "请选择“好施”的目标，将一半手牌（向下取整）交给该角色",
	["~haoshi"] = "选择需要交出的手牌→选择一名其他角色→点击确定",
	["haoshi:yes"] = "额外摸两张牌：若你拥有五张或更多的手牌，你将一半数量（向下取整）的手牌交给除你外场上手牌数最少的一名角色",
	["#Dimeng"] = "%from (原来 %arg 手牌) 与 %to (原来 %arg2 手牌) 交换了手牌",

	["#jiaxu"] = "冷酷的毒士",
	["jiaxu"] = "贾诩",
	["wansha"] = "完杀",
	[":wansha"] = "<font color=\"blue\"><b>锁定技。</b></font>你的回合内，除你和濒死角色外的角色不能使用【桃】。",
	["weimu"] = "帷幕",
	[":weimu"] = "<font color=\"blue\"><b>锁定技。</b></font>你不能被选择为黑色锦囊的目标。",
	["luanwu"] = "乱武",
	[":luanwu"] = "<font color=\"red\"><b>限定技。</b></font>出牌阶段，你可以令所有其他角色选择一项：对距离最近的另一名角色使用一张【杀】，或失去1点体力。",
	["@chaos"] = "乱武",
	["#WanshaOne"] = "%from 的“%arg”被触发，只能 %from 自救",
	["#WanshaTwo"] = "%from 的“%arg”被触发，只有 %from 和 %to 才能救 %to",
	["@luanwu-slash"] = "请使用一张【杀】响应“乱武”",

	["#dongzhuo"] = "魔王",
	["dongzhuo"] = "董卓",
	["jiuchi"] = "酒池",
	[":jiuchi"] = "你可以将一张♠手牌当【酒】使用。",
	["roulin"] = "肉林",
	[":roulin"] = "<font color=\"blue\"><b>锁定技。</b></font>每当你指定女性角色为【杀】的目标后，或女性角色指定你为【杀】的目标后，目标角色须连续使用两张【闪】抵消此【杀】。",
	["benghuai"] = "崩坏",
	[":benghuai"] = "<font color=\"blue\"><b>锁定技。</b></font>回合结束阶段开始时，若你的体力值不为场上最少（或之一），你须选择一项：失去1点体力，或失去1点体力上限。",
	["baonue"] = "暴虐",
	[":baonue"] = "<font color=\"orange\"><b>主公技。</b></font>其他群雄角色造成一次伤害后，该角色可以进行一次判定：若判定结果为♠，你回复1点体力。",
	[":benghuai:"] = "请选择一项:",
	["benghuai:hp"] = "体力",
	["benghuai:maxhp"] = "体力上限",
	["baonue:yes"] = "你可以进行一次判定：若判定结果为♠，董卓回复1点体力",

-- Lines
--曹丕
	["$xingshang1"] = "来，管杀还管埋！",
	["$xingshang2"] = "我的是我的！你的，还是我的！",
	["$fangzhu1"] = "给我翻过来！",
	["$fangzhu2"] = "死罪可免，活罪难赦！",
	["$songwei1"] = "仙福永享，寿与天齐！",
	["$songwei2"] = "千秋万载，一统江山！",
	["~caopi"] = "子建…子建…",

--徐晃
	["$duanliang1"] = "截其源，断其粮，贼可擒也！",
	["$duanliang2"] = "人是铁，饭是钢。",
	["~xuhuang"] = "一顿不吃，饿的慌…",

--孟获
	["$zaiqi1"] = "起~~~~~~~~~~",
	["$zaiqi2"] = "丞相助我！",
	["$huoshou1"] = "背黑锅我来，送死，你去！",
	["$huoshou2"] = "统统算我的！",
	["~menghuo"] = "七纵之恩，来世…再…报…了！",

--祝融
	["$juxiang1"] = "小小把戏！",
	["$juxiang2"] = "大王，看我的！",
	["$lieren1"] = "尝尝我飞刀的厉害！",
	["$lieren2"] = "亮兵器吧~",
	["~zhurong"] = "大王…我，先走一步了…",

--孙坚
	["$yinghun1"] = "以吾魂魄，保佑吾儿之基业。",
	["$yinghun2"] = "不诛此贼三族！则吾死不瞑目！",
	["~sunjian"] = "有埋伏额…啊！~~",

--鲁肃
	["$haoshi1"] = "拿去拿去，莫跟哥哥客气！",
	["$haoshi2"] = "来来来，见面分一半",
	["$dimeng1"] = "以和为贵，以和为贵。",
	["$dimeng2"] = "合纵连横，方能以弱胜强。",
	["~lusu"] = "此联盟一破，吴蜀休矣…",

--贾诩
	["$wansha1"] = "神仙难救！神仙难救啊！",
	["$wansha2"] = "我要你三更死，谁敢留你到五更！",
	["$weimu1"] = "此计伤不到我！",
	["$weimu2"] = "你奈我何？",
	["$luanwu1"] = "哭喊吧！哀求吧！挣扎吧！然后死吧！",
	["$luanwu2"] = "哼哼哼，坐山观虎斗！",
	["$LuanwuAnimate"] = "anim=image/animate/luanwu.png",
	["~jiaxu"] = "我的时辰…也…到…了……",

--董卓
	["$jiuchi1"] = "呃呵呵，再来……一壶！",
	["$jiuchi2"] = "呃呵呵呵呵，好酒好酒！",
	["$roulin1"] = "美人儿，来，香一个！！",
	["$roulin2"] = "食色，性也！",
	["$benghuai1"] = "呃~~",
	["$benghuai2"] = "哎，我是不是该减肥了？",
	["$baonue1"] = "顺我者昌，逆我者亡！",
	["$baonue2"] = "呵呵哈哈哈哈哈哈！",
	["~dongzhuo"] = "汉室衰落，非我一人之罪…",


-- illustrator
	["illustrator:caopi"] = "SoniaTang",
	["illustrator:lusu"] = "LiuHeng",
	["illustrator:sunjian"] = "LiuHeng",
	["illustrator:menghuo"] = "废柴男",
	["illustrator:zhurong"] = "废柴男",
	["illustrator:dongzhuo"] = "小冷",
	["illustrator:xuhuang"] = "Tuu.",
}