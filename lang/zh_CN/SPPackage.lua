-- translation for SP Package

return {
	["sp"] = "SP",

	["#yangxiu"] = "恃才放旷",
	["yangxiu"] = "杨修",
	["illustrator:yangxiu"] = "张可",
	["jilei"] = "鸡肋",
	["danlao"] = "啖酪",
	[":jilei"] = "每当你受到一次伤害时，你可以选择一种牌的类别，伤害来源不能使用、打出或弃置其该类别的手牌，直到回合结束。",
	[":danlao"] = "每当一张锦囊牌指定了包括你在内的至少两名目标时，你可以摸一张牌，然后该锦囊牌对你无效。",
	["$jilei1"] = "食之无肉，弃之有味", 
	["$jilei2"] = "曹公之意我已了然", 	
	["$danlao1"] = "来来，一人一口", 
	["$danlao2"] = "我喜欢", 	
	["~yangxiu"] = "我固自以死之晚也",
	["#DanlaoAvoid"] = "“%arg2”效果被触发，【%arg】 对 %from 无效",
	["#Jilei"] = "%from 对 %to 发动“<font color=\"yellow\"><b>鸡肋</b></font>”，限制了 %arg",
	["#JileiClear"] = "%from 的“<font color=\"yellow\"><b>鸡肋</b></font>”效果消失",
	["@jilei_basic"] = "鸡肋（基本牌）",
	["@jilei_equip"] = "鸡肋（装备牌）",
	["@jilei_trick"] = "鸡肋（锦囊牌）",

	["#gongsunzan"] = "白马将军",
	["gongsunzan"] = "公孙瓒",
	["illustrator:gongsunzan"] = "Vincent",
	["yicong"] = "义从",
	[":yicong"] = "<font color=\"blue\"><b>锁定技。</b></font>若你的体力值大于2，你与其他角色的距离-1；若你的体力值小于或等于2，其他角色与你的距离+1。",
	["$yicong1"] = "冲啊！",
	["$yicong2"] = "众将听令，摆好阵势，御敌！",
	["~gongsunzan"] = "我军将败，我已无颜苟活于世",

	["#yuanshu"] = "仲家帝",
	["yuanshu"] = "袁术",
	["illustrator:yuanshu"] = "吴昊",
	["yongsi"] = "庸肆",
	[":yongsi"] = "<font color=\"blue\"><b>锁定技。</b></font>摸牌阶段，你额外摸X张牌。弃牌阶段开始时，你须弃置X张牌。（X为现存势力数）",
	["weidi"] = "伪帝",
	[":weidi"] = "<font color=\"blue\"><b>锁定技。</b></font>你拥有且可以发动当前主公的主公技。",
	["cv_yuanshu:convert"] = "你可以替换为台版袁术",
	["$yongsi1"] = "玉玺在手，天下我有",
	["$yongsi2"] = "大汉天下，已半入我手",
	["$weidi1"] = "我才是皇帝！",
	["$weidi2"] = "你们都得听我的号令！",
	["~yuanshu"] = "可恶，就差一步了！",
	["#YongsiGood"] = "%from 的“%arg2”被触发，额外摸了 %arg 张牌",
	["#YongsiBad"] = "%from 的“%arg2”被触发，须弃置 %arg 张牌",
	["#YongsiJilei"] = "%from 的“%arg2”被触发，由于“<font color=\"yellow\"><b>鸡肋</b></font>”的效果，仅弃置了 %arg 张牌", 
	["#YongsiWorst"] = "%from 的“%arg2”被触发，弃置了所有牌（共 %arg 张）",

	["#sp_diaochan"] = "绝世的舞姬",
	["sp_diaochan"] = "SP貂蝉",
	["&sp_diaochan"] = "貂蝉",
	["illustrator:sp_diaochan"] = "巴萨小马",
	["cv_diaochan:convert"] = "你可以替换为SP貂蝉或台版貂蝉或国战貂蝉",
	["cv_diaochan"] = "SP替换",
	["cv_diaochan:sp_diaochan"] = "SP",
	["cv_diaochan:tw_diaochan"] = "台湾版SP",
	["cv_diaochan:heg_diaochan"] = "国战版",

	["#sp_sunshangxiang"] = "梦醉良缘",
	["sp_sunshangxiang"] = "SP孙尚香",
	["&sp_sunshangxiang"] = "孙尚香",
	["illustrator:sp_sunshangxiang"] = "木美人",
	["cv_sunshangxiang:convert"] = "你可以替换为蜀势力SP孙尚香",

	["#sp_guanyu"] = "汉寿亭侯",
	["sp_guanyu"] = "SP关羽",
	["&sp_guanyu"] = "关羽",
	["illustrator:sp_guanyu"] = "LiuHeng",
	["cv:sp_guanyu"] = "喵小林，官方",
	["danji"] = "单骑",
	[":danji"] = "<font color=\"purple\"><b>觉醒技。</b></font>回合开始阶段开始时，若你的手牌数大于体力值，且本局游戏主公为曹操，你失去1点体力上限，然后获得技能“马术”。",
	["#DanjiWake"] = "%from 的手牌数(%arg)大于体力值(%arg2)，且本局游戏主公为曹操，触发“<font color=\"yellow\"><b>单骑</b></font>”觉醒",
	["$danji"] = "吾兄待我甚厚，誓以共死，今往投之，望曹公见谅。",
	["$DanjiAnimate"] = "anim=image/animate/danji.png",

	["#sp_caiwenji"] = "金璧之才",
	["sp_caiwenji"] = "SP蔡文姬",
	["&sp_caiwenji"] = "蔡文姬",
	["illustrator:sp_caiwenji"] = "木美人",
	["cv_caiwenji:convert"] = "你可以替换为魏势力SP蔡文姬",
	["cv:sp_caiwenji"] = "呼呼",
	
	["#sp_jiaxu"] = "算无遗策",
	["sp_jiaxu"] = "SP贾诩",
	["&sp_jiaxu"] = "贾诩",
	["illustrator:sp_jiaxu"] = "雪君S",
	["cv_jiaxu:convert"] = "你可以替换为魏势力SP贾诩",
	["$LuanwuAnimateSP"] = "anim=image/animate/luanwuSP.png",
	
	["#sp_pangde"] = "枱榇之悟",
	["sp_pangde"] = "SP庞德",
	["&sp_pangde"] = "庞德",
	["illustrator:sp_pangde"] = "天空之城",
	["cv_pangde:convert"] = "你可以替换为魏势力SP庞德",

	["#sp_machao"] = "西凉的猛狮",
	["sp_machao"] = "SP马超",
	["&sp_machao"] = "马超",
	["illustrator:sp_machao"] = "天空之城",
	["cv_machao:convert"] = "你可以替换为群雄势力SP马超",
	
	["#caohong"] = "福将",
	["caohong"] = "曹洪",
	["illustrator:caohong"] = "LiuHeng",
	["cv:caohong"] = "喵小林",
	["yuanhu"] = "援护",
	[":yuanhu"] = "回合结束阶段开始时，你可以将一张装备牌置于一名角色装备区内：若此牌为武器牌，你弃置该角色距离1的一名角色区域内的一张牌；若此牌为防具牌，该角色摸一张牌；若此牌为坐骑牌，该角色回复1点体力。",
	["@yuanhu-equip"] = "你可以发动“援护”",
	["~yuanhu"] = "选择一张装备牌→选择一名角色→点击确定",
	["$yuanhu1"] = "持吾兵戈，随我杀敌！", --武器
	["$yuanhu2"] = "汝今势微，吾当助汝。", --防具
	["$yuanhu3"] = "公急上马，洪敌贼军！", --坐骑
	["$yuanhu4"] = "天下可无洪，不可无公。", --对曹操
	["$yuanhu5"] = "持戈整兵，列阵御敌！", --对自己
	["~caohong"] = "主公已安，洪纵死亦何惜……", 
	
	["#guanyinping"] = "武姬",
	["guanyinping"] = "关银屏",
	["illustrator:guanyinping"] = "木美人",
	["cv:guanyinping"] = "蒲小猫",
	["xueji"] = "血祭",
	[":xueji"] = "<font color=\"green\"><b>阶段技。</b></font>出牌阶段，你可以弃置一张红色牌并选择你攻击范围内的至多X名其他角色：若如此做，你对这些角色各造成1点伤害，然后这些角色各摸一张牌。（X为你已损失的体力值）",
	["huxiao"] = "虎啸",
	[":huxiao"] = "<font color=\"blue\"><b>锁定技。</b></font>若你于出牌阶段使用【杀】被【闪】抵消，本阶段你可以额外使用一张【杀】。",
	["wuji"] = "武继",
	[":wuji"] = "<font color=\"purple\"><b>觉醒技。</b></font>回合结束阶段开始时，若你于此回合内造成了至少3点伤害，你增加1点体力上限，回复1点体力，然后失去技能“虎啸”。",
	["#WujiWake"] = "%from 本回合已造成 %arg 点伤害，触发“%arg2”觉醒",
	["$WujiAnimate"] = "anim=image/animate/wuji.png",
	["$xueji1"] = "陷坚摧锋，以报家仇！",
	["$xueji2"] = "踏平南土，以消国恨！",
	["$huxiao1"] = "若无后手，何言为将？",
	["$huxiao2"] = "刀行如流水，杀你个措手不及！",
	["$wuji"] = "武圣虽死，血脉尚存。先父佑我，再现武魂！",
	["~guanyinping"] = "父亲……",
	
	["#chenlin"] = "破竹之咒",
	["chenlin"] = "陈琳",
	["illustrator:chenlin"] = "木美人",
	["bifa"] = "笔伐",
	[":bifa"] = "回合结束阶段开始时，你可以将一张手牌移出游戏并选择一名其他角色：若如此做，该角色的回合开始时，观看该牌，然后可以交给你一张与该牌类型相同的牌并获得该牌，否则将该牌置入弃牌堆，然后失去1点体力。",
	["@bifa-remove"] = "你可以发动“笔伐”",
	["~bifa"] = "选择一张手牌→选择一名其他角色→点击确定",
	["@bifa-give"] = "请交给目标角色一张类型相同的手牌",
	["songci"] = "颂词",
	[":songci"] = "出牌阶段，你可以令一名手牌数大于体力值的角色弃置两张牌，或令一名手牌数小于体力值的角色摸两张牌。对每名角色限一次。",
	["@songci"] = "颂词",
	["cv:chenlin"] = "苏遥东",
	["$bifa1"] = "文人岂无用，笔墨亦作兵。",
	["$bifa2"] = "将军可否直视此言？",                   --get
	["$bifa3"] = "行文如刀，笔墨诛心！",                 --throw
	["$songci1"] = "广宜恩信，班扬符赏。",               --drawCards
	["$songci2"] = "汝众违旅叛，当有此报！",             --discardCards
	["~chenlin"] = "文未达意，贼不伏诛，吾之过也……",
	
	["#tw_diaochan"] = "绝世的舞姬",
	["tw_diaochan"] = "台版貂蝉",
	["&tw_diaochan"] = "貂蝉",
	["illustrator:tw_diaochan"] = "陳俊佐",
	
	["#tw_yuanshu"] = "仲家帝",
	["tw_yuanshu"] = "台版袁术",
	["&tw_yuanshu"] = "袁术", 
	["illustrator:tw_yuanshu"] = "湯翔麟",
	
	["#tw_zhaoyun"] = "少年将军",
	["tw_zhaoyun"] = "台版赵云",
	["&tw_zhaoyun"] = "赵云", 
	["illustrator:tw_zhaoyun"] = "湯翔麟",
	["cv_zhaoyun:convert"] = "你可以替换为台版赵云",
	
	["#tw_daqiao"] = "矜持之花",
	["tw_daqiao"] = "台版大乔",
	["&tw_daqiao"] = "大乔", 
	["illustrator:tw_daqiao"] = "玄兔",
	["cv_daqiao:convert"] = "你可以替换为王战大乔或台版大乔",
	["cv_daqiao"] = "SP替换",
	["cv_daqiao:wz_daqiao"] = "王战版",
	["cv_daqiao:tw_daqiao"] = "台湾版",
	
	["#wz_daqiao"] = "矜持之花",
	["wz_daqiao"] = "王战大乔",
	["&wz_daqiao"] = "大乔", 
	["illustrator:wz_daqiao"] = "Natsu",
	
	["#wz_xiaoqiao"] = "矫情之花",
	["wz_xiaoqiao"] = "王战小乔",
	["&wz_xiaoqiao"] = "小乔", 
	["illustrator:wz_xiaoqiao"] = "Natsu",
	["cv_xiaoqiao:convert"] = "你可以替换为王战小乔或国战小乔",
	["cv_xiaoqiao"] = "SP替换",
	["cv_xiaoqiao:wz_xiaoqiao"] = "王战版",
	["cv_xiaoqiao:heg_xiaoqiao"] = "国战版",
	
--hulao mode
	["Hulaopass"] = "虎牢关模式",

	["#shenlvbu1"] = "最强神话",
	["shenlvbu1"] = "吕布-虎牢关",
	["&shenlvbu1"] = "最强神话",
	["illustrator:shenlvbu1"] = "LiuHeng",
	["#shenlvbu2"] = "暴怒的战神",
	["shenlvbu2"] = "吕布-虎牢关",
	["&shenlvbu2"] = "暴怒战神",
	["illustrator:shenlvbu2"] = "LiuHeng",
	["xiuluo"] = "修罗",
	[":xiuluo"] = "回合开始阶段开始时，你可以弃置一张与判定区内延时类锦囊牌花色相同的手牌：若如此做，你弃置该延时类锦囊牌。",
	["@xiuluo"] = "请弃置一张花色相同的手牌",
	["shenwei"] = "神威",
	[":shenwei"] = "<font color=\"blue\"><b>锁定技。</b></font>摸牌阶段，你额外摸两张牌。你的手牌上限+2。",
	["shenji"] = "神戟",
	[":shenji"] = "若你的装备区没有武器牌，每当你使用【杀】时，你可以额外选择至多两个目标。",

	["#Reforming"] = "%from 进入重整状态",
	["#ReformingRecover"] = "%from 在重整状态中回复了1点体力",
	["#ReformingDraw"] = "%from 在重整状态中摸了一张牌",
	["#ReformingRevive"] = "%from 从重整状态中复活!",
	["draw_1v3"] = "重整摸牌",
	["weapon_recast"] = "武器重铸",
	["Hulaopass:recover"] = "回复1点体力",
	["Hulaopass:draw"] = "摸一张牌",
	
	["cv:shenlvbu2"] = "风叹息",
	["$shenwei"] = "飞将之威，上天亦知！",
	["$shenji"] = "神戟在手，何人能及！",
	["$xiuluo"] = "哼！鬼蜮伎俩，休想阻我！",
	["~shenlvbu2"] = "什么？我败了？！",
	["$StageChangeAnimate"] = "anim=image/animate/StageChange.png",
	
--sp_card
	["sp_cards"] = "SP卡牌包",
	["sp_moonspear"] = "银月枪",
	[":sp_moonspear"] = "装备牌·武器<br />攻击范围：３<br />武器特效：你的回合外，每当你使用或打出一张黑色手牌时，你可以令你攻击范围内的一名其他角色选择一项：打出一张【闪】，或受到到你对其造成的1点伤害。",
	["@moon-spear-jink"] = "【银月枪】效果被触发，请打出一张【闪】",

--hegemony_sp
	["hegemony_sp"] = "国战SP",
	
	["#sp_heg_zhouyu"] = "大都督",
	["sp_heg_zhouyu"] = "国战SP周瑜",
	["&sp_heg_zhouyu"] = "周瑜",
	["illustrator:sp_heg_zhouyu"] = "牧童的短笛",
	["cv_zhouyu:convert"] = "你可以替换为国战周瑜或台国战SP周瑜",
	["cv_zhouyu"] = "SP替换",
	["cv_zhouyu:heg_zhouyu"] = "国战版",
	["cv_zhouyu:sp_heg_zhouyu"] = "国战SP版",
}
