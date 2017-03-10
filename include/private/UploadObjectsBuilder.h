/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_UPLOADOBJECTSBUILDER_H_
#define PRISM_UPLOADOBJECTSBUILDER_H_

#include "UploadTaskQueuer.h"
#include "JsonConfig.h"
#include "OutputController.h"

namespace prism
{
namespace connect
{

class UploadObjectsBuilder
{
public:
    UploadObjectsBuilder();
//    UploadObjectsBuilder(const JsonConfigPtr cfg);

    UploadTaskQueuerPtr makeUploadTaskQueuer() const;
    // may block for several seconds due to interaction with the server
    OutputControllerPtr makeOutputController() const;

private:
//    JsonConfigPtr cfg_;
    UploadQueuePtr artifactUploadQueue_;

    void makeArtifactUploadQueue();
};

} // namespace connect
} // namespace prism

#endif // PRISM_UPLOADOBJECTSBUILDER_H_
