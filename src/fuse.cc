#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <string>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>


#include "swagger.h"
#include "service_manager.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static const boost::filesystem::path root_path{"/"};

static const std::vector<std::string> special_files
  = {"REQUEST", "RESPONSE", "REQUEST_HEADERS", "RESPONSE_HEADERS"};

using namespace swaggerfs;

static int swaggerfs_getattr(const char *c_path, struct stat *stbuf) {

  int res = 0;

  memset(stbuf, 0, sizeof(struct stat));

  boost::filesystem::path path{c_path};

  /**
   * Extract swaggerfs service definition from fuse context
   */
  struct fuse_context *ctx = fuse_get_context();

  service* service_definition
    = static_cast<service*>(ctx->private_data);

  /**
   * Check the length of the path and based on it either handle
   * root directory attributes or 
   */
  if(boost::filesystem::equivalent(root_path, path)) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  }
  else {
    /**
     * The path is longer then root - let's tokenize it
     */
    std::vector<std::string> path_tokens;
    
    boost::split(path_tokens, path.string(), boost::is_any_of("/"));

    std::vector<swagger::tag> tags 
      = swaggerfs::swagger_parser::get_tags(service_definition->model);

    /**
     * If the request path is longer
     */
    if(path_tokens.size() >= 2) {
      /**
       * Check if the first element of the path is a valid Swagger 'tag'
       */
      if(std::any_of(tags.begin(), tags.end(), 
                    [&](swagger::tag t){ return t.name == path_tokens[1]; })) {

        if(path_tokens.size() == 2)
          stbuf->st_mode = S_IFDIR | 0755;
          stbuf->st_nlink = 3;
        }
        else {
          /**
           * Check if second element of the path is a valid Swagger 'operation'
           */
          std::vector<swagger::operation> operations 
            = swaggerfs::swagger_parser::get_operations(service_definition->model,
                                                        path_tokens[1]);

          if(std::any_of(operations.begin(), operations.end(), 
              [&](swagger::operation o){ return o.id == path_tokens[2]; })) {
            
            if(path_tokens.size() == 3) {
              stbuf->st_mode = S_IFDIR | 0755;
              stbuf->st_nlink = 3;
            }
            else {
              /**
               * Check if third element of the path is a valid special file
               */
              if(std::any_of(special_files.begin(), special_files.end(), 
                         [&](std::string f){ return f == path_tokens[3]; })) {

                stbuf->st_mode = S_IFREG | 0640;
                stbuf->st_nlink = 1;
              }
              else {
                res = -ENOENT;
              }
            }
          }
          else {
            res = -ENOENT;
          }

        }
      }
      else {
        res = -ENOENT;
      }

    }

  return res;
}

static int swaggerfs_readdir(const char *c_path, void *buf, 
                             fuse_fill_dir_t filler, off_t offset, 
                             struct fuse_file_info *fi)
{
  (void) offset;
  (void) fi;


  boost::filesystem::path path{c_path};

  /**
   * Extract swaggerfs service definition from fuse context
   */
  struct fuse_context *ctx = fuse_get_context();

  service* service_definition
    = static_cast<service*>(ctx->private_data);

  std::vector<swagger::tag> tags 
    = swaggerfs::swagger_parser::get_tags(service_definition->model);

  /**
   * Check the length of the path and based on it either handle
   * root directory attributes or 
   */
  if(boost::filesystem::equivalent(root_path, path)) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    /**
     * List tags in service
     */
    for(swagger::tag t : tags) {
      struct stat st;
      memset(&st, 0, sizeof(st));
      filler(buf, t.name.c_str(), &st, 0);
    }

  }
  else {
    /**
     * The path is longer then root - let's tokenize it
     */
    std::vector<std::string> path_tokens;
    boost::split(path_tokens, path.string(), boost::is_any_of("/"));

    if(path_tokens.size() == 2) {
      /**
       * List operations in a tag
       */
      std::vector<swagger::operation> operations 
        = swaggerfs::swagger_parser::get_operations(service_definition->model,
                                                    path_tokens[1]);
      filler(buf, ".", NULL, 0);
      filler(buf, "..", NULL, 0);
      for(swagger::operation o : operations) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        filler(buf, o.id.c_str(), &st, 0);
      }
    }
    else if(path_tokens.size() == 3) {
      /**
       * List special request files in operation
       */
      filler(buf, ".", NULL, 0);
      filler(buf, "..", NULL, 0);
      for(std::string special_file : special_files) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        filler(buf, special_file.c_str(), &st, 0);
      }
    }

  }

  return 0;
}

static int swaggerfs_open(const char *path, struct fuse_file_info *fi)
{
  //if (strcmp(path, hello_path) != 0)
  //  return -ENOENT;

  if ((fi->flags & 3) != O_RDONLY)
    return -EACCES;

  return 0;
}

static int swaggerfs_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{

  /*
  size_t len;
  (void) fi;
  if(strcmp(path, hello_path) != 0)
    return -ENOENT;

  len = strlen(hello_str);
  if (offset < len) {
    if (offset + size > len)
      size = len - offset;
    memcpy(buf, hello_str + offset, size);
  } else
    size = 0;
  */
 
  return 0;
}

struct fuse_operations swaggerfs_operations = {
  .getattr  = swaggerfs_getattr,
  .readdir  = swaggerfs_readdir,
  .open     = swaggerfs_open,
  .read     = swaggerfs_read,
} ;

