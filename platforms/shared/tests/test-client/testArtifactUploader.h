/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef TEST_ARTIFACT_UPLOADER_H
#define TEST_ARTIFACT_UPLOADER_H

#include <string>

namespace prism
{
namespace test
{

void testArtifactUploader(const std::string& apiRoot,
                          const std::string& apiToken,
                          const std::string& cameraName);

} // namespace test
} // namespace prism

#endif // #ifndef TEST_ARTIFACT_UPLOADER_H
