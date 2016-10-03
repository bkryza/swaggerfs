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
#include "http_client.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static const boost::filesystem::path __root_path{"/"};

/**
 * These buffers hold current buffers for each operation
 */
static std::map<std::string, std::string> __request_buffers;
static std::map<std::string, std::string> __response_buffers;
// static std::map<std::string, std::vector<std::string, std::string> > 
//                                                             __request_headers;
// static std::map<std::string, std::vector<std::string, std::string> > 
//                                                             __response_headers;


static const std::vector<std::string> __special_files
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
  service* service_definition = static_cast<service*>(ctx->private_data);

  /**
   * Check the length of the path and based on it either handle
   * root directory attributes or 
   */
  if(boost::filesystem::equivalent(__root_path, path)) {
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

        if(path_tokens.size() == 2) {
          stbuf->st_mode = S_IFDIR | 0755;
          stbuf->st_nlink = 3;
        }
        else {
          /**
           * Check if second element of the path is a valid Swagger 'operation'
           */
          if(swaggerfs::swagger_parser::has_operation(service_definition->model,
                                                      path_tokens[1], 
                                                      path_tokens[2])) {
            
            if(path_tokens.size() == 3) {
              stbuf->st_mode = S_IFDIR | 0755;
              stbuf->st_nlink = 3;
            }
            else {
              /**
               * Check if third element of the path is a valid special file
               */
              swaggerfs::swagger::operation op
                = swaggerfs::swagger_parser::get_operation(service_definition->model,
                                                           path_tokens[1], 
                                                           path_tokens[2]);
              std::string request_file;
              auto it = std::find(__special_files.begin(), 
                                  __special_files.end(), 
                                  path_tokens[3]);

              if(it != __special_files.end()) {

                std::string special_file = *it;

                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1;
                if(special_file == "REQUEST" && 
                   op.method == http::method::POST) {
                  /**
                   * Fill the request buffer with example if available
                   */
                  

                }
                else {
                  stbuf->st_size = 0;
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
      }
      else {
        res = -ENOENT;
      }
    }
  }

  return res;
}

static int swaggerfs_readdir(const char *c_path, void *buf, 
                             fuse_fill_dir_t filler, off_t offset, 
                             struct fuse_file_info *fi) {

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
  if(boost::filesystem::equivalent(__root_path, path)) {
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
      for(std::string special_file : __special_files) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        filler(buf, special_file.c_str(), &st, 0);
      }
    }

  }

  return 0;
}

static int swaggerfs_open(const char *c_path, struct fuse_file_info *fi) {

  /**
   * Extract swaggerfs service definition from fuse context
   */
  struct fuse_context *ctx = fuse_get_context();

  service* service_definition = static_cast<service*>(ctx->private_data);
  
  /**
   * Parse the path
   */
  boost::filesystem::path path{c_path};
  std::vector<std::string> path_tokens;
  boost::split(path_tokens, path.string(), boost::is_any_of("/"));

  if(path_tokens.size()<4) {
    return -ENOENT;
  }

  std::vector<swagger::tag> tags 
    = swaggerfs::swagger_parser::get_tags(service_definition->model);

  if(!std::any_of(tags.begin(), tags.end(), 
                      [&](swagger::tag t){ return t.name == path_tokens[1]; })) {
    /**
     * No such tag
     */
    return -ENOENT;
  }



  return 0;

}

static int swaggerfs_read(const char *c_path, char *buf, size_t size, 
                          off_t offset, struct fuse_file_info *fi) {

  int res = 0;

  boost::filesystem::path path{c_path};

  /**
   * Open should only work on actual files
   */
  std::vector<std::string> path_tokens;
  boost::split(path_tokens, path.string(), boost::is_any_of("/"));

  if(path_tokens.size() < 4) {
    /**
     * We can only read from files which have a path:
     * /{TAG}/{OPERATION}/{SPECIAL_FILE}
     */
    return -ENOENT;
  }

  /**
   * Extract swaggerfs service definition from fuse context
   */
  struct fuse_context *ctx = fuse_get_context();
  service* service_definition = static_cast<service*>(ctx->private_data);
  const char* hello_str = "hello";

  size_t len = strlen(hello_str);
  if (offset < len) {
    if (offset + size > len) {
      size = len - offset;
    }
    memcpy(buf, hello_str + offset, size);
  } 
  else {
    size = 0;
  }

  return size;

}

struct fuse_operations swaggerfs_operations = {
  .getattr  = swaggerfs_getattr,
  .readdir  = swaggerfs_readdir,
  .open     = swaggerfs_open,
  .read     = swaggerfs_read,
} ;

