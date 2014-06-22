//============================================================================
// Name        : enumToString2.cpp
// Author      : OC
// Version     :
// Copyright   : GPL
// Description : Generate C code from .h file to convert enum to
//               their string symbols
//============================================================================
// v0.1 initial version
// v0.2 quick move to C++ to be able to use map for context management
//============================================================================
// TODO:
// - write generated code to a filename
// - add cmd line parser
//============================================================================

#include "config.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <string>

/* define Branch prediction hints macros for GCC 4.0.0 and upper */
#if (GCC_VERSION > 40000) /* GCC 4.0.0 */
#ifndef likely
#define likely(x)   __builtin_expect(!!(x),1)
#endif /* likely */
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x),0)
#endif /* unlikely */
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif /* GCC 4.0.0 */

struct NameSpace {
     std::string name;
     std::string fullname;
};

typedef std::map<std::string,NameSpace> NameSpaces;

static NameSpaces namespaces;

static void XMLCALL startNodeHandler(void *data, const char *node, const char **attributs)
{
     int i;
     if (strcmp(node,"Enumeration") == 0) {
          std::string Name;
          std::string Context;
          for (i = 0; attributs[i]; i += 2) {
               if (strcmp(attributs[i],"name") == 0) {
                    Name = attributs[i+1];
               } else if (strcmp(attributs[i],"context") == 0) {
                    Context = attributs[i+1];
               }
          }
          std::map<std::string,NameSpace>::const_iterator i = namespaces.find(Context);
          if (i != namespaces.end()) {
               printf("const char *to_string(const enum %s::%s e) {\n"
                      "switch(e) {\n"
                      ,i->second.fullname.c_str(),Name.c_str());
               //std::clog << "context " << Context << " found" << std::endl;
          } else {
               printf("const char *to_string(const enum %s e) {\n"
                      "switch(e) {\n"
                      ,Name.c_str());
               //std::clog << "context " << Context << " not found" << std::endl;
          }
     } else if (strcmp(node,"EnumValue") == 0) {
          for (i = 0; attributs[i]; i += 2) {
               if (strcmp(attributs[i],"name") == 0) {
                    printf("case %s: return \"%s\";\n",attributs[i+1],attributs[i+1]);
               }
          }
     } else if (strcmp(node,"Namespace") == 0) {
          std::pair<std::string,NameSpace> newNameSpace;

          for (i = 0; attributs[i]; i += 2) {
               if (strcmp(attributs[i],"name") == 0) {
                    if (strcmp(attributs[i+1],"::") != 0) {
                         newNameSpace.second.name = attributs[i+1];
                    } else {
                         newNameSpace.second.name = "";
                    }
               } else if (strcmp(attributs[i],"demangled") == 0) {
                    if (strcmp(attributs[i+1],"::") != 0) {
                         newNameSpace.second.fullname = attributs[i+1];
                    } else {
                         newNameSpace.second.fullname = "";
                    }
               } else if (strcmp(attributs[i],"id") == 0) {
                    newNameSpace.first = attributs[i+1];
               }
          }
          namespaces.insert(newNameSpace);
          //std::clog << "new namespace " << newNameSpace.second.fullname << " context " << newNameSpace.first << std::endl;
     }
}

static void XMLCALL endNodeHandler(void *data, const char *node)
{
     //xmlData->Depth--;
     if (strcmp(node,"Enumeration") == 0) {
          printf("} //switch(e)\n");
          printf("}\n");
     }
}

static inline size_t getFileSize(int fd)
{
     size_t size = 0;
     struct stat buffer;
     if (fstat(fd,&buffer) == 0) {
          size = buffer.st_size;
     } else {
          const int error = errno;
          fprintf(stderr,"error fstat error %d (%m)",error);
     }
     return size;
}

static int readXMLFile(const char *filename)
{
     int error = EXIT_SUCCESS;
     int fd = open(filename, O_RDONLY);
     if (fd != -1) {
          error = posix_fadvise(fd,0,0,POSIX_FADV_SEQUENTIAL);
          if (unlikely(error)) {
               fprintf(stderr,"warning posix_fadvise POSIX_FADV_SEQUENTIAL error %d\n",error);
          }
          error = posix_fadvise(fd,0,0,POSIX_FADV_WILLNEED);
          if (unlikely(error)) {
               fprintf(stderr,"warning posix_fadvise POSIX_FADV_WILLNEED error %d\n",error);
          }
          error = posix_fadvise(fd,0,0,POSIX_FADV_NOREUSE);
          if (unlikely(error)) {
               fprintf(stderr,"warning posix_fadvise POSIX_FADV_NOREUSE error %d\n",error);
          }
          const size_t size = getFileSize(fd);
          if (size != 0) {
               const char *content = (const char *)malloc(size);
               if (content != NULL) {
                    const ssize_t n = read(fd, (void*)content, size);
                    if (n > 0) {
                         XML_Parser parser = XML_ParserCreate(NULL);
                         if (parser != NULL) {
                              XML_SetElementHandler(parser, startNodeHandler, endNodeHandler);
                              if (XML_Parse(parser, content, n, 1) == XML_STATUS_ERROR) {
                                   fprintf(stderr, "Parse error at line %lu:\n%s\n",
                                           XML_GetCurrentLineNumber(parser),
                                           XML_ErrorString(XML_GetErrorCode(parser)));
                              }
                              XML_ParserFree(parser);
                         } else {
                              error = ENOMEM;
                              fprintf(stderr,"Couldn't allocate memory for the XML parser\n");
                         }
                    } else {
                         error = errno;
                         fprintf(stderr,"Error %d reading file %s (%m)\n", error, filename);
                    }
                    free((void*)content);
                    content = NULL;
               } else {
                    fprintf(stderr,"failed to allocate %zu bytes of memory to store the file %s 's content\n",size,filename);
               }
          } else {
               /* error already printed */
               error = -1;
          }
     } else {
          error = errno;
          fprintf(stderr,"error %d opening file %s (%m)\n", error, filename);
     }
     return error;
}

int main(int argc, char *argv[])
{
     int error = EXIT_SUCCESS;
     error = readXMLFile(argv[1]); //TODO: add cmd line parser
     return error;
}
