switch(statevar) 
{
	case 1: //move to goal
		if (no_pink_is_seen)
		{
			move_towards_x,y;
			if(sensor[113] < set_value)
			{
				statevar = 2;
			}
			if(sensor on the right < set_value)
			{
				statevar = 3;
			}
			if(sensor on the left < set_value)
			{
				statevar = 4;
			}
			if((sensor[112]<=set_value)&&(sensor[112]>set_value)&&(sensor[114]<=set_value))
			{
				statevar = 5;
			}
		}
		else
		{
			statevar = 6;
		}
		break;
	
	case 2: // front obstacle avoidance
		if (no_pink_is_seen)
		{
			left or right_wall_follows;
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
		
	case 3: // right obstacle avoidance
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
		
	case 4: // left obstacle avoidance
		if (no_pink_is_seen)
		{
			left_wall_follows;
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
		
	case 5: // find legs
		brake the car; 
		if (no obstacle is found)
		{
			statevar = 1;
		}
		break;
			
	case 6: //move to pink
		move_towards_pink;
		if (can't_see_pink)
		{
			statevar = 1;
		}
		break;
}
