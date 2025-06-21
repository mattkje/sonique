//
// Created by Matti Kjellstadli on 21/06/2025.
//

#include "FileUtils.h"


std::string GetResourcePath(const std::string &filename) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *) path, PATH_MAX)) {
        std::string fullPath = std::string(path) + "/" + filename;
        CFRelease(resourcesURL);
        return fullPath;
    }
    CFRelease(resourcesURL);
    return filename; // fallback
}