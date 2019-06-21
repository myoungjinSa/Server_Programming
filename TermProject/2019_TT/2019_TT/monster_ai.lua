
myid = 9999


function set_uid( id )
	myid = id;
end


function event_player_move( player )
	player_x = API_get_x(player);
	player_y = API_get_y(player);
	my_x = API_get_x(myid);
	my_y = API_get_y(myid);

	local ret = false;

	if(player_x == my_x) then
		if(player_y == my_y) then
			API_SendMessage(player,myid,"HELLO");
			ret = true;
		end
	end
	return ret;
end

function say_bye()

	
	API_SendMessage(player,myid,"BYE");
	
end

function set_player_dir( direction )
	if(direction == 1) then
		print("south");
		npc_direction = direction; 
	end
	if(direction == 2) then
		print("north");
		npc_direction = direction;
	end
	if(direction == 3) then
		print("east");
		npc_direction = direction;
	end
	if(direction == 4) then
		print("west");
		npc_direction = direction;
	end
end


function move_npc(move_count) 
	my_x = API_get_x(myid);
	my_y = API_get_y(myid);

	
	
	if(move_count < 3) then
		if(npc_direction == 1) then
			
			my_y = my_y - 1;
			move_count = move_count + 1;
		end
		if(npc_direction == 2) then
			
			my_y = my_y + 1;
			move_count = move_count + 1;
		end
		if(npc_direction == 3) then
			
			my_x = my_x - 1;
			move_count = move_count + 1;
		end
		if(npc_direction == 4) then
			
			my_x = my_x + 1;
			move_count = move_count + 1;
		end
	end
	return my_x,my_y,move_count;
end
