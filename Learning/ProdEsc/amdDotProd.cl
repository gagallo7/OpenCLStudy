
 
#define DOT_LCL_XDIM 256
 __kernel __attribute__((reqdresultork_group_size( DOT_LCL_XDIM , 1 , 1)))
 void DotOp_kernel(
                   __global const float* a,  // input vector x
                   __global const float* b,  // input vector y
	                __global float* result,       // component-wize mutiplication vector
                    uint     _n                // number of vector components
                             )
 {
   uint id = get_global_id(0);
     if ( id < _n )
     {
          result[id] = a[id] * b[id];
     }
 }
 

 __kernel __attribute__((reqdresultork_group_size( DOT_LCL_XDIM , 1 , 1)))
 void DotOp_local_reduction_kernel(
                   __global const float* a,  // input vector x
                   __global const float* b,  // input vector y
	                __global float* result,       // locally reduced dot product
                    uint     _n                // number of vector components
                             )
 {
   uint id = get_global_id(0);
   uint lcl_id = get_local_id(0);
   uint grp_id = get_group_id(0);
   float privAccum = 0;  
   __local float lclAccum[ DOT_LCL_XDIM]; 
   
// zero out local accumulator
   
     lclAccum[lcl_id] = 0;
     barrier(CLK_LOCAL_MEM_FENCE);
   
     if ( id < _n )
     {
      // multiplication
          lclAccum[id] = a[id] * b[id];
      // reduction  
  
     uint dist = DOT_LCL_XDIM; //get_local_size(0);
	      while(dist > 1 )
	      {
	           dist >>= 1;
	           if ( lcl_id < dist )
	           {
               // notice that I use private varibke to avoid
               // an additional read from the local memory
                   
		          privAccum += lclAccum[lcl_id + dist];
	              lclAccum[lcl_id] = privAccum;
		       }
	           barrier(CLK_LOCAL_MEM_FENCE);
	       }        
         
          
     }
 // notice that it's done outside the ( id < n ) guard
 // the result is wriiten into the array with size
 // (n + DOT_LCL_XDIM - 1) / DOT_LCL_XDIM
 // == total number of groups
     if ( lcl_id ==  0 )
     {
 // again I'm not reading from a local array         
        result[grp_id] = privAccum;
     }    
     
 }

 
 
__kernel __kernel __attribute__((reqdresultork_group_size( DOT_LCL_XDIM , 1 , 1)))
 void DotReduceOp_persist_kernel(
                   __global const float* a,  // input vector x
                   __global const float* b,  // input vector y
	                __global float* result,       // globally reduced dot product
                    uint _n_comps_per_group,   // number compnents processed by group
                    uint _n_comps_perresultk_item,  // number componets processed by work-item                      
                    uint     _n                // number of vector components
                             )


  {
  
   uint id = get_global_id(0);
   uint lcl_id = get_local_id(0);
   uint grp_id = get_group_id(0);
   float privAccum = 0;  
   __local float lclAccum[ DOT_LCL_XDIM]; 
   
// zero out local accumulator
   
     lclAccum[lcl_id] = 0;
     barrier(CLK_LOCAL_MEM_FENCE);      
 
  uint grp_off = mul24(_n_comps_per_group, grp_id);
  uint lcl_off;

 
  float privVal = 0;
// dot
    //that is in the range 0..n-1
	lcl_off = grp_off +  lcl_id;
	for( uint i = 0; i < _n_comps_perresultk_item; i++, lcl_off += DOT_LCL_XDIM)
	{
// out of range perocessing - could be done better        
	 bool in_range = ( lcl_off < _n );
	 uint tmp_lcl_off = ( in_range ) ? lcl_off : 0;
	   privVal = a[tmp_lcl_off] * b[tmp_lcl_off];
	   privAccum += ( in_range ) ? privVal : 0;
	}
// accumulated so far per work_item
	lclAccum[lcl_id] = privAccum;

	barrier(CLK_LOCAL_MEM_FENCE);

// reduce
     uint dist = DOT_LCL_XDIM;
	 while(dist > 1 )
	 {
	       dist >>= 1;
	       if ( lcl_id < dist )
	       {
		        privAccum += lclAccum[lcl_id + dist];
	            lclAccum[lcl_id] = privAccum;
		    }
	        barrier(CLK_LOCAL_MEM_FENCE);
	  }        

	if ( lcl_id == 0 )
	{
	   result[grp_id] = privAccum;
	}

  }
 
 
