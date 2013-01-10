sgs.ai_skill_use["@@drzhiheng"] = function(self, prompt)
	if self.player:getHandcardNum() > self.player:getHp() then return "." end
	local to_discard = {}
	for _, zcard in sgs.qlist(self.player:getHandcards()) do
		if not zcard:isKindOf("Peach") and not zcard:isKindOf("Jink") and not zcard:isKindOf("Nullification") then
			table.insert(to_discard, zcard:getId())
		end
	end
	for index = #to_discard, 1, -1 do
		if self.player:isJilei(sgs.Sanguosha:getCard(to_discard[index])) then table.remove(to_discard, index) end
	end
	if #to_discard>0 then 
		return ("@DrZhihengCard="..table.concat(to_discard,"+"))
	else
		return ("@DrZhihengCard=.")
	end
end

local drjiuyuanv_skill = {}
drjiuyuanv_skill.name = "drjiuyuanv"
table.insert(sgs.ai_skills, drjiuyuanv_skill)
drjiuyuanv_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("ForbidDrJiuyuan") then return end
	if self.player:isKongcheng() or self.player:getHandcardNum() < self.player:getHp() - 1 or self.player:getKingdom() ~= "wu" then return end

	return sgs.Card_Parse("@DrJiuyuanCard=.")
end

sgs.ai_skill_use_func.DrJiuyuanCard = function(card,use,self)
	local targets = {}
	for _, player in ipairs(self.friends_noself) do
		if player:hasLordSkill("drjiuyuan") and not player:hasFlag("DrJiuyuanInvoked") and not player:hasSkill("manjuan") then
			table.insert(targets, player)
		end
	end

	if #targets == 0 then return end
	if self:needBear() then return "." end
	self:sort(targets, "defense")
	for _, lord in ipairs(targets) do
		local card_str
		local cards = self.player:getHandcards()
		local card_id = -1
		
		self:sortByKeepValue(cards)
		for _, acard in sgs.qlist(cards) do
			if lord:isWeak() and (acard:isKindOf("Peach") or acard:isKindOf("Jink")) then 
				card_id = acard:getEffectiveId()
				break
			end
		end
		if card_id == -1 then
			for _, acard in sgs.qlist(cards) do
				if self:getKeepValue(acard) < 7 then
					card_id = acard:getEffectiveId()
					break
				end
			end
		end

		if card_id ~= -1 then
			if self:isFriend(lord) and not (lord:hasSkill("kongcheng") and lord:isKongcheng()) then
				card_str = "@DrJiuyuanCard=" .. card_id
			end

			if card_str then
				use.card = sgs.Card_Parse(card_str)
				if use.to then use.to:append(lord) end
				return
			end
		end
	end
end

sgs.ai_card_intention.DrJiuyuanCard = -80

sgs.ai_use_priority.DrJiuyuanCard = 3
sgs.ai_use_value.DrJiuyuanCard = 8.5

local drjiedao_skill = {}
drjiedao_skill.name = "drjiedao"
table.insert(sgs.ai_skills, drjiedao_skill)
drjiedao_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("DrJiedaoCard") then return end
	local can_use = false
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if p:getWeapon() then
			can_use = true
			break
		end
	end
	if can_use then return sgs.Card_Parse("@DrJiedaoCard=.") end
end

sgs.ai_skill_use_func.DrJiedaoCard = function(card,use,self)
	self:sort(self.enemies, "defense")
	local targets = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:getWeapon() and not self:hasSkills(sgs.lose_equip_skill, enemy) and not enemy:hasSkill("tuntian") then 
			table.insert(targets, enemy)
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() then 
			table.insert(targets, friend)
		end
	end
	if #targets == 0 then return end
	local equip_need
	for _, enemy in ipairs(self.enemies) do
		if self:getCardsNum("Slash") > 0 
			and self.player:canSlash(enemy, nil, false) and not self:cantbeHurt(enemy) and sgs.isGoodTarget(enemy, self.enemies)
			and self:damageIsEffective(self.player, sgs.DamageStruct_Normal, enemy) then
			if self.player:distanceTo(enemy) == 1 and self:getCardsNum("Slash") > 3 then equip_need = "Crossbow"
			elseif self.player:distanceTo(enemy) <= 2 and enemy:getArmor() and not enemy:getArmor():isKindOf("SilverLion") then equip_need = "QinggangSword"
			elseif self.player:distanceTo(enemy) <= 2 and enemy:isKongcheng() then equip_need = "GudingBlade"
			elseif self.player:distanceTo(enemy) <= 4 and self:isEquip("Vine", enemy) then equip_need = "Fan"
			elseif self.player:distanceTo(enemy) <= 5 and (enemy:getOffensiveHorse() or enemy:getDefensiveHorse()) then equip_need = "KylinBow"
			elseif self.player:getHandcardNum() == 1 and self.player:getHandcards():first():isKindOf("Slash") then equip_need = "Halberd"
			elseif self.player:distanceTo(enemy) <= 3 and self.player:getCardCount(true) >= 5 then equip_need = "Axe"
			end
		end
		if equip_need then break end
	end
	if equip_need then
		for _, player in ipairs(targets) do
			if player:getWeapon() and player:getWeapon():isKindOf(equip_need) then
				use.card = card
				if use.to then use.to:append(player) end
				return
			end
		end
	end
	if self.player:getWeapon() then return end
	for _, player in ipairs(targets) do
		if player:getWeapon() then
			use.card = card
			if use.to then use.to:append(player) end
			return
		end
	end
end

sgs.ai_skill_cardask["@JijiuDecrease"] = function(self, data)
	local damage = data:toDamage()
	if not self:isFriend(damage.to) then return "." end
	if self:hasSkills(sgs.masochism_skill, damage.to) and damage.damage <= 1 and damage.to:getHp() > 1 then return "." end
	local cards=sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if card:isRed() then return "$" .. card:getEffectiveId() end
	end
	return "."
end

local drqingnang_skill = {}
drqingnang_skill.name = "drqingnang"
table.insert(sgs.ai_skills, drqingnang_skill)
drqingnang_skill.getTurnUseCard = function(self)
	if self.player:isNude() or self.player:getLostHp() == 0 then return nil end

	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)

	self:sortByKeepValue(cards)

	local card_str = ("@DrQingnangCard=%d"):format(cards[1]:getId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.DrQingnangCard = function(card,use,self)
	use.card = card
end

sgs.ai_use_priority.DrQingnangCard = 4.2
sgs.ai_card_intention.DrQingnangCard = -80

sgs.ai_skill_discard.drwushuang = function(self, discard_num, min_num, optional, include_equip)
	local lvbu = self.room:findPlayerBySkillName("drwushuang")
	if not lvbu or lvbu:hasSkill("wuyan") then return {} end
	local cards = self.player:getCards("he")
	local to_discard = {}
	cards = sgs.QList2Table(cards)
	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_PlaceEquip then
			if card:isKindOf("SilverLion") and self.player:isWounded() then return -2
			elseif card:isKindOf("OffensiveHorse") then return 1
			elseif card:isKindOf("Weapon") then return 2
			elseif card:isKindOf("DefensiveHorse") then return 3
			elseif card:isKindOf("Armor") then return 4 
			end
		elseif self:hasSkills(sgs.lose_equip_skill) then return 5
		else return 0 
		end
	end
	local compare_func = function(a, b)
		if aux_func(a) ~= aux_func(b) then return aux_func(a) < aux_func(b) end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end

	table.sort(cards, compare_func)
	local can_discard
	for _, card in ipairs(cards) do
		if not self.player:isJilei(card) then table.insert(can_discard, card) end
	end
	if #can_discard > 0 then
		table.insert(to_discard, can_discard[1]:getEffectiveId())
		return to_discard
	end
	return {}
end
-- FORMATTED