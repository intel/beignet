typedef struct{
  int a;
  int b;
}mystruct;

__kernel void compiler_pipe_convenience_write_int(write_only pipe int p, __global int *src)
{
    int gid = get_global_id(0);
    write_pipe(p, &src[gid]);
}
__kernel void compiler_pipe_convenience_read_int(read_only pipe int p, __global int *dst)
{
    int gid = get_global_id(0);
    read_pipe(p, &dst[gid]);
}
__kernel void compiler_pipe_convenience_write_mystruct(write_only pipe mystruct p, __global mystruct *src)
{
    int gid = get_global_id(0);
    write_pipe(p, &src[gid]);
}
__kernel void compiler_pipe_convenience_read_mystruct(read_only pipe mystruct p, __global mystruct *dst)
{
    int gid = get_global_id(0);
    read_pipe(p, &dst[gid]);
}

__kernel void compiler_pipe_reserve_write_int(write_only pipe int p, __global int *src)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = reserve_write_pipe(p, 1);
    if(is_valid_reserve_id(res_id))
    {
      write_pipe(p, res_id, 0, &src[gid]);
      commit_write_pipe(p, res_id);
    }
}
__kernel void compiler_pipe_reserve_read_int(read_only pipe int p, __global int *dst)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = reserve_read_pipe(p, 1);
    if(is_valid_reserve_id(res_id))
    {
      read_pipe(p, res_id, 0, &dst[gid]);
      commit_read_pipe(p, res_id);
    }
}
__kernel void compiler_pipe_reserve_write_mystruct(write_only pipe mystruct p, __global mystruct *src)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = reserve_write_pipe(p, 1);
    if(is_valid_reserve_id(res_id))
    {
      write_pipe(p, res_id, 0, &src[gid]);
      commit_write_pipe(p, res_id);
    }
}
__kernel void compiler_pipe_reserve_read_mystruct(read_only pipe mystruct p, __global mystruct *dst)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = reserve_read_pipe(p, 1);
    if(is_valid_reserve_id(res_id))
    {
      read_pipe(p, res_id, 0, &dst[gid]);
      commit_read_pipe(p, res_id);
    }
}

__kernel void compiler_pipe_workgroup_write_int(write_only pipe int p, __global int *src)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = work_group_reserve_write_pipe(p, get_local_size(0));
    if(is_valid_reserve_id(res_id))
    {
      write_pipe(p, res_id, get_local_id(0), &src[gid]);
      work_group_commit_write_pipe(p, res_id);
    }
}
__kernel void compiler_pipe_workgroup_read_int(read_only pipe int p, __global int *dst)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = work_group_reserve_read_pipe(p, get_local_size(0));
    if(is_valid_reserve_id(res_id))
    {
      read_pipe(p, res_id, get_local_id(0), &dst[gid]);
      work_group_commit_read_pipe(p, res_id);
    }
}
__kernel void compiler_pipe_workgroup_write_mystruct(write_only pipe mystruct p, __global mystruct *src)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = work_group_reserve_write_pipe(p, get_local_size(0));
    if(is_valid_reserve_id(res_id))
    {
      write_pipe(p, res_id, get_local_id(0), &src[gid]);
      work_group_commit_write_pipe(p, res_id);
    }
}
__kernel void compiler_pipe_workgroup_read_mystruct(read_only pipe mystruct p, __global mystruct *dst)
{
    int gid = get_global_id(0);
    reserve_id_t res_id = work_group_reserve_read_pipe(p, get_local_size(0));
    if(is_valid_reserve_id(res_id))
    {
      read_pipe(p, res_id, get_local_id(0), &dst[gid]);
      work_group_commit_read_pipe(p, res_id);
    }
}

__kernel void compiler_pipe_query(write_only pipe int p, __global uint *src)
{
    int gid = get_global_id(0);
    write_pipe(p,&gid);
    if(gid == 0) {
      src[0] = get_pipe_num_packets(p);
      src[1] = get_pipe_max_packets(p);
    }
}
