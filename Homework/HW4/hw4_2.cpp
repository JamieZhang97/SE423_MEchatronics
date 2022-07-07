void RGBtoHSV(unsigned char r, unsigned char g, unsigned char b, unsigned char *h, unsigned char *s, unsigned char *v);
{	
	float max;
	float mid;
	float min;
	
	float r_unit;
	float g_unit;
	float b_unit;
	
	float h_unit;
	float s_unit;
	float v_unit;	
	
	r_unit = r/255.0;
	g_unit = g/255.0;
	b_unit = b/255.0; 
	
	
	
	if(r_unit>g_unit)
	{
		if(g_unit>b_unit)
		{
			max = (float)r_unit;
			mid = (float)g_unit;
			min = (float)b_unit;
		}
		else
		{
			if(b_unit>r_unit)
			{
			max = (float)b_unit;
			mid = (float)r_unit;
			min = (float)g_unit;
			} 
			else
			{
			max = (float)r_unit;
			mid = (float)b_unit;
			min = (float)g_unit;
			}
		}
	} 
	else
	{
		if(g_unit<b_unit)
		{
			max = (float)b_unit;
			mid = (float)g_unit;
			min = (float)r_unit;
		}
		else
		{
			if(b_unit<r_unit)
			{
				max = (float)g_unit;
				mid = (float)r_unit;
				min = (float)b_unit;
			} 
			else
			{
			max = (float)g_unit;
			mid = (float)b_unit;
			min = (float)r_unit;	
			}
		}
	}
	
	if (max==min)
	{
		h_unit = (unsigned char)0;
	}
	else if(max==r_unit && g_unit>=b_unit)
	{
		h_unit = (unsigned char)(60*(g_unit - b_unit)/(max - min));
	}
	else if(max==r_unit && g<b)
	{
		h_unit = (unsigned char)(60*(g_unit - b_unit)/(max - min) + 360);
	}
	else if(max==g_unit)
	{
		h_unit = (unsigned char)(60*(b_unit - r_unit)/(max - min) + 120);
	}
	else if(max==b_unit)
	{
		h_unit = (unsigned char)(60*(g_unit - b_unit)/(max - min) + 240);
	}
	
	if(max==0)
	{
		s_unit =  (unsigned char)0;
	} 
	else
	{
		s_unit = (unsigned char)(1-min/max);
	}
	
	v_unit = (unsigned char)max;
	
	*h = h_unit*255/360;
	*s = s_unit*255;
	*v = v_unit*255;
	
} 
