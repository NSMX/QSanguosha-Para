sgs.ai_skill_invoke.xingshang = true

sgs.ai_skill_use["@@fangzhu"] = function(self, prompt)
	local friends = {}
	for _, frd in ipairs(self.friends_noself) do
		if not (frd:hasSkill("manjuan") and frd:getPhase() == sgs.Player_NotActive) then
			table.insert(friends, frd)
		end
	end
	self:sort(friends)
	local target
	if #friends > 0 then
		for _, friend in ipairs(friends) do
			if friend:isAlive() then
				if self.player:getLostHp() > 1 and friend:hasSkill("jijiu") then
					target = friend
					break
				end
				if not friend:faceUp() then
					target = friend
					break
				end
				if (friend:hasSkill("jushou") or friend:hasSkill("kuiwei")) and friend:getPhase() == sgs.Player_Play then
					target = friend
					break
				end
			end
		end
	end

	if not target then
		local x = self.player:getLostHp()
		if x >= 3 and #friends > 0 then
			target = friends[1]
		else
			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if enemy:isAlive() and enemy:faceUp() 
				   and enemy:hasSkill("manjuan") and enemy:getPhase() == sgs.Player_NotActive then
					target = enemy
					break
				end
			end
			if not target then
				for _, enemy in ipairs(self.enemies) do
					local invoke = true
					if (self:hasSkills("jushou|neojushou|kuiwei", enemy)) and enemy:getPhase() == sgs.Player_Play then invoke = false end
					if enemy:hasSkill("shenfen") and enemy:getMark("@wrath") >= 6 and enemy:getPhase() == sgs.Player_Play 
						and (not enemy:hasUsed("ShenfenCard") or enemy:hasFlag("ShenfenUsing")) then invoke = false end
					if enemy:hasSkill("lihun") and enemy:getPhase() == sgs.Player_Play and not enemy:hasUsed("LihunCard") then invoke = false end
					if enemy:hasSkill("jijiu") and x >= 2 then invoke = false end
					if enemy:isAlive() and enemy:faceUp() and invoke then
						target = enemy
						break
					end
				end
			end
		end
	end

	if target then
		return "@FangzhuCard=.->" .. target:objectName()
	else
		return "."
	end
end

sgs.ai_skill_invoke.songwei = function(self, data)
	for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if p:hasLordSkill("songwei") and self:isFriend(p) and not p:hasFlag("songwei_used") and p:isAlive() then
			return true
		end
	end
end

sgs.ai_skill_playerchosen.songwei = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, target in ipairs(targets) do
		if self:isFriend(target) and not target:hasFlag("songwei_used") and target:isAlive() then 
			return target 
		end 
	end
	return targets[1]
end

sgs.ai_card_intention.FangzhuCard = function(card, from, tos)
	if from:getLostHp() < 3 then
		sgs.updateIntention(from, tos[1], 80/from:getLostHp())
	end
end

sgs.ai_need_damaged.fangzhu = function (self, attacker)
	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if not friend:faceUp() then
			return true
		end
		if (friend:hasSkill("jushou") or friend:hasSkill("kuiwei")) and friend:getPhase() == sgs.Player_Play then
			return true
		end
	end	
	if self.player:getLostHp()<=1 and sgs.turncount>2 then return true end	
	return false
end

sgs.ai_chaofeng.caopi = -3

duanliang_skill={}
duanliang_skill.name="duanliang"
table.insert(sgs.ai_skills,duanliang_skill)
duanliang_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if (acard:isBlack()) and (acard:isKindOf("BasicCard") or acard:isKindOf("EquipCard")) and (self:getDynamicUsePriority(acard)<sgs.ai_use_value.SupplyShortage)then
			card = acard
			break
		end
	end

	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("supply_shortage:duanliang[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)

	assert(skillcard)

	return skillcard

end

sgs.xuhuang_suit_value = 
{
	spade = 3.9,
	club = 3.9
}

sgs.ai_chaofeng.xuhuang = 4

sgs.ai_skill_invoke.zaiqi = function(self, data)
	return self.player:getLostHp() >= 2
end

sgs.ai_skill_invoke.lieren = function(self, data)
	local damage = data:toDamage()
	if self:isEnemy(damage.to) then
		if self.player:getHandcardNum()>=self.player:getHp() then return true
		else return false
		end
	end

	return false
end

function sgs.ai_skill_pindian.lieren(minusecard, self, requestor)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if requestor:objectName() == self.player:objectName() then
		return cards[1]:getId()
	end
	return self:getMaxCard(self.player):getId()
end

sgs.ai_skill_choice.yinghun = function(self, choices)
	return self.yinghunchoice
end

sgs.ai_skill_use["@@yinghun"] = function(self, prompt)
	local x = self.player:getLostHp()
	if x == 1 and #self.friends == 1 then
		for _, enemy in ipairs(self.enemies) do
			if enemy:hasSkill("manjuan") then 
				return "@YinghunCard=.->" .. enemy:objectName()
			end
		end
		return "."
	end

	if #self.friends > 1 then
		for _, friend in ipairs(self.friends_noself) do
			if self:hasSkills(sgs.lose_equip_skill, friend) and friend:isAlive() then
				self.yinghun = friend
				self.yinghunchoice = "dxt1"
				break
			end
		end
		self:sort(self.friends_noself, "chaofeng")
		for _, afriend in ipairs(self.friends_noself) do
			if not afriend:hasSkill("manjuan") and afriend:isAlive() then self.yinghun = afriend end
		end
		if self.yinghun and not self.yinghunchoice then self.yinghunchoice = "dxt1" end
	else
		self:sort(self.enemies, "handcard")
		for index = #self.enemies, 1, -1 do
			local enemy = self.enemies[index]
			if enemy:isAlive() and not enemy:isNude() and not (self:hasSkills(sgs.lose_equip_skill, enemy) and
			   not (enemy:getCards("he"):length() < x or sgs.getDefense(enemy) < 3)) then
				self.yinghun = enemy
				self.yinghunchoice = "d1tx"
				break
			end
		end
	end

	if self.yinghun then
		return "@YinghunCard=.->" .. self.yinghun:objectName()
	else
		return "."
	end
end

local function getLowerBoundOfHandcard(self)
	local least = math.huge
	local players = self.room:getOtherPlayers(self.player)
	for _, player in sgs.qlist(players) do
		least = math.min(player:getHandcardNum(), least)
	end

	return least
end

local function getBeggar(self)
	local least = getLowerBoundOfHandcard(self)

	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() == least then
			return friend
		end
	end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getHandcardNum() == least then
			return player
		end
	end
end

sgs.ai_skill_invoke.haoshi = function(self, data)
	if self.player:getHandcardNum() <= 1 and not self.player:hasSkill("yongsi") then
		return true
	end

	local beggar = getBeggar(self)
	return self:isFriend(beggar) and not beggar:hasSkill("manjuan")
end

sgs.ai_skill_use["@@haoshi!"] = function(self, prompt)
	local beggar = getBeggar(self)

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	local card_ids = {}
	for i=1, math.floor(#cards/2) do
		table.insert(card_ids, cards[i]:getEffectiveId())
	end

	return "@HaoshiCard=" .. table.concat(card_ids, "+") .. "->" .. beggar:objectName()
end

sgs.ai_card_intention.HaoshiCard = -80

dimeng_skill={}
dimeng_skill.name="dimeng"
table.insert(sgs.ai_skills,dimeng_skill)
dimeng_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("DimengCard") then return end
	card=sgs.Card_Parse("@DimengCard=.")
	return card

end

sgs.ai_skill_use_func.DimengCard=function(card,use,self)
	local cardNum = 0
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if not self.player:isJilei(c) then cardNum = cardNum + 1 end
	end
	for _, c in sgs.qlist(self.player:getEquips()) do
		if not self.player:isJilei(c) then cardNum = cardNum + 1 end
	end

	self:sort(self.enemies,"handcard")
	local friends={}
	for _,player in ipairs(self.friends_noself) do
		if not player:hasSkill("manjuan") then
			table.insert(friends, player)
		end
	end
	self:sort(friends,"handcard")

	local lowest_friend=friends[1]

	self:sort(self.enemies,"defense")
	if lowest_friend then
		local hand2=lowest_friend:getHandcardNum()
		for _,enemy in ipairs(self.enemies) do
			local hand1=enemy:getHandcardNum()

			if enemy:hasSkill("manjuan") and (hand1 > hand2 - 1) and (hand1-hand2)<=cardNum then
				use.card=card
				if use.to then
					use.to:append(enemy)
					use.to:append(lowest_friend)
				end
				return
			end
		end
		for _,enemy in ipairs(self.enemies) do
			local hand1=enemy:getHandcardNum()

			if (hand1 > hand2) then
				if (hand1-hand2)<=cardNum then
					use.card=card
					if use.to then
						use.to:append(enemy)
						use.to:append(lowest_friend)
					end
					return
				end
			end
		end
	end
end

sgs.ai_card_intention.DimengCard = function(card, from, to)
	local compare_func = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end
	table.sort(to, compare_func)
	if to[1]:getHandcardNum() < to[2]:getHandcardNum() then
		sgs.updateIntention(from, to[1], (to[2]:getHandcardNum()-to[1]:getHandcardNum())*20+40)
	end
end

sgs.ai_use_value.DimengCard = 3.5
sgs.ai_use_priority.DimengCard = 2.3

sgs.dynamic_value.control_card.DimengCard = true

sgs.ai_chaofeng.lusu = 4

luanwu_skill={}
luanwu_skill.name="luanwu"
table.insert(sgs.ai_skills, luanwu_skill)
luanwu_skill.getTurnUseCard=function(self)
	if self.player:getMark("@chaos") <= 0 then return end
	local good, bad = 0, 0
	local lord = self.room:getLord()
	if self.role ~= "rebel" and self:isWeak(lord) then return end
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isWeak(player) then
			if self:isFriend(player) then bad = bad + 1
			else good = good + 1
			end
		end
	end
	if good == 0 then return end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if getCardsNum("Analeptic", player) > 0 then
			if self:isFriend(player) then good = good + 1.0/player:getHp()
			else bad = bad + 1.0/player:getHp()
			end
		end

		local has_slash = (getCardsNum("Slash", player) > 0)
		local can_slash = false
		if not can_slash then
			for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
				if player:distanceTo(p) <= player:getAttackRange() then can_slash = true break end
			end
		end
		if not has_slash or not can_slash then
			if self:isFriend(player) then good = good + math.max(getCardsNum("Peach", player), 1)
			else bad = bad + math.max(getCardsNum("Peach", player), 1)
			end
		end

		if getCardsNum("Jink", player) == 0 then
			local lost_value = 0
			if self:hasSkills(sgs.masochism_skill, player) then lost_value = player:getHp()/2 end
			if self:isFriend(player) then bad = bad + (lost_value+1)/player:getHp()
			else good = good + (lost_value+1)/player:getHp()
			end
		end
	end

	if good > bad then return sgs.Card_Parse("@LuanwuCard=.") end
end

sgs.ai_skill_use_func.LuanwuCard=function(card,use,self)
	use.card = card
end

sgs.ai_skill_playerchosen.luanwu = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.dynamic_value.damage_card.LuanwuCard = true

jiuchi_skill={}
jiuchi_skill.name="jiuchi"
table.insert(sgs.ai_skills,jiuchi_skill)
jiuchi_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Spade) then --and (self:getUseValue(acard)<sgs.ai_use_value.Analeptic) then
			card = acard
			break
		end
	end

	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("analeptic:jiuchi[spade:%s]=%d"):format(number, card_id)
	local analeptic = sgs.Card_Parse(card_str)

	assert(analeptic)

	return analeptic

end

sgs.ai_view_as.jiuchi = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_PlaceEquip then
		if card:getSuit() == sgs.Card_Spade then
			return ("analeptic:jiuchi[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

sgs.ai_skill_invoke.baonue = function(self, data)
	for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if p:hasLordSkill("baonue") and self:isFriend(p) and not p:hasFlag("baonue_used") and p:isAlive() then
			return true
		end
	end
end

sgs.ai_skill_playerchosen.baonue = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, target in ipairs(targets) do
		if self:isFriend(target) and not target:hasFlag("baonue_used") and target:isAlive() then 
			return target 
		end 
	end
	return targets[1]
end

sgs.dongzhuo_suit_value = 
{
	spade = 5,
}