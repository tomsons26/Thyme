#include "intersect.h"

void IntersectionClass::Append_Object_Array(
	int max_count, 
	int &current_count, 
	RenderObjClass **obj_array,
	RenderObjClass *obj)
{
	if(current_count < max_count) {
		obj_array[current_count] = obj;
		current_count++;
		return;
	}
	//DEBUG_SAY(("IntersectionClass::Append_Object_Array - Too many objects\n"));
}
