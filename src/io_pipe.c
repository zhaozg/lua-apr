/* Pipe I/O handling module for the Lua/APR binding.
 *
 * Author: Peter Odding <peter@peterodding.com>
 * Last Change: October 18, 2010
 * Homepage: http://peterodding.com/code/lua/apr/
 * License: MIT
 */

#include "lua_apr.h"
#include <apr_file_io.h>

static int pipe_open(lua_State*, lua_apr_pipe_f);

/* apr.pipe_open_stdin() -> pipe {{{1
 *
 * Open standard input as a pipe. On success the pipe is returned, otherwise a
 * nil followed by an error message is returned. 
 */

int lua_apr_pipe_open_stdin(lua_State *L)
{
  return pipe_open(L, apr_file_open_stdin);
}

/* apr.pipe_open_stdout() -> pipe {{{1
 *
 * Open standard output as a pipe. On success the pipe is returned, otherwise a
 * nil followed by an error message is returned.
 */

int lua_apr_pipe_open_stdout(lua_State *L)
{
  return pipe_open(L, apr_file_open_stdout);
}

/* apr.pipe_open_stderr() -> pipe {{{1
 *
 * Open standard error as a pipe. On success the pipe is returned, otherwise a
 * nil followed by an error message is returned. 
 */

int lua_apr_pipe_open_stderr(lua_State *L)
{
  return pipe_open(L, apr_file_open_stderr);
}

/* apr.namedpipe_create(name [, permissions]) -> status {{{1
 *
 * Create a [named pipe] [named_pipe]. On success true is returned, otherwise a
 * nil followed by an error message is returned. See the documentation on
 * permissions for the optional second argument.
 *
 * Named pipes can be used for interprocess communication:
 *
 * 1. Check if the named pipe already exists, if it doesn't then create it
 * 2. Have each process access the named pipe using `apr.file_open()`
 * 3. Communicate between the two processes over the read/write ends of the
 *    named pipe and close it when the communication is finished.
 *
 * Note that APR supports named pipes on UNIX but not on Windows. If you try
 * anyhow the error message "This function has not been implemented on this
 * platform" is returned.
 *
 * [named_pipe]: http://en.wikipedia.org/wiki/Named_pipe
 */

int lua_apr_namedpipe_create(lua_State *L)
{
  apr_status_t status;
  apr_pool_t *pool;
  apr_fileperms_t permissions;
  const char *filename;

  pool = to_pool(L);
  filename = luaL_checkstring(L, 1);
  permissions = check_permissions(L, 2, 0);
  status = apr_file_namedpipe_create(filename, permissions, pool);
  return push_status(L, status);
}

/* apr.pipe_create() -> input, output {{{1
 *
 * Create an [anonymous pipe] [anon_pipe]. On success the write and read ends
 * of the pipe are returned, otherwise a nil followed by an error message is
 * returned.
 *
 * [anon_pipe]: http://en.wikipedia.org/wiki/Anonymous_pipe
 */

int lua_apr_pipe_create(lua_State *L)
{
  apr_status_t status;
  lua_apr_file *input, *output;
  lua_apr_pool *refpool;

  /* XXX The apr_file_pipe_create() API enforces that both pipes are allocated
   * from the same memory pool. This means we need a reference counted memory
   * pool to avoid double free bugs on exit!
   */
  refpool = refpool_alloc(L);
  input = file_alloc(L, NULL, refpool);
  output = file_alloc(L, NULL, refpool);
  status = apr_file_pipe_create(&input->handle, &output->handle, refpool->ptr);
  if (status != APR_SUCCESS)
    return push_error_status(L, status);
  input->input.object = input->handle;
  output->output.object = output->handle;
  return 2;
}

/* }}}1 */

int pipe_open(lua_State *L, lua_apr_pipe_f open_std_pipe)
{
  apr_status_t status;
  lua_apr_file *pipe;

  pipe = file_alloc(L, NULL, NULL);
  status = open_std_pipe(&pipe->handle, pipe->pool->ptr);
  if (status != APR_SUCCESS)
    return push_error_status(L, status);
  /* Initialize the buffer associated with the pipe. */
  init_buffers(L, &pipe->input, &pipe->output, pipe->handle, 1,
      (lua_apr_buf_rf) apr_file_read,
      (lua_apr_buf_wf) apr_file_write,
      (lua_apr_buf_ff) apr_file_flush);

  return 1;
}

/* vim: set ts=2 sw=2 et tw=79 fen fdm=marker : */
