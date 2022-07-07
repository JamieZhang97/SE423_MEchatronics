switch(statevar) 
{
	case 1:
		if (no_pink_is_seen)
		{
			move_towards_x,y;
			if(front_distance < set_value)
			{
				statevar = 2;
			}
		}
		else
		{
			statevar = 3;
		}
		break;
	
	case 2:
		if (no_pink_is_seen)
		{
			right_wall_follows;
			if(front_distance > set_value)
			{
				statevar = 1;
			}
		}
		else
		{
			statevar = 3;
		}
		break;
			
	case 3:
		move_towards_pink;
		if (can't_see_pink)
		{
			statevar = 1;
		}
		break;
}
