/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "TestHelper.h"

#include "../utils/PropertiesUtils.h"
#include "../utils/UrlUtils.h"

#include <gtest/gtest.h>


class SmoothTreeTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    m_reprChooser = new CTestRepresentationChooserDefault();
    tree = new SmoothTestTree();
  }

  void TearDown() override
  {
    testHelper::effectiveUrl.clear();
    delete tree;
    tree = nullptr;
    delete m_reprChooser;
    m_reprChooser = nullptr;
  }

  void OpenTestFile(std::string filePath) { OpenTestFile(filePath, "http://foo.bar/" + filePath); }

  void OpenTestFile(std::string filePath, std::string url) { OpenTestFile(filePath, url, {}); }

  void OpenTestFile(std::string filePath,
                    std::string url,
                    std::map<std::string, std::string> manifestHeaders)
  {
    testHelper::testFile = filePath;

    // Download the manifest
    UTILS::CURL::HTTPResponse resp;
    if (!testHelper::DownloadFile(url, {}, {}, resp))
    {
      LOG::Log(LOGERROR, "Cannot download \"%s\" DASH manifest file.", url.c_str());
      exit(1);
    }

    m_reprChooser->Initialize(m_kodiProps.m_chooserProps);
    // We set the download speed to calculate the initial network bandwidth
    m_reprChooser->SetDownloadSpeed(500000);

    tree->Configure(m_kodiProps, m_reprChooser, "urn:uuid:EDEF8BA9-79D6-4ACE-A3C8-27DCD51D21ED", "");

    // Parse the manifest
    if (!tree->Open(resp.effectiveUrl, resp.headers, resp.data))
    {
      LOG::Log(LOGERROR, "Cannot open \"%s\" Smooth Streaming manifest.", url.c_str());
      exit(1);
    }
  }

  SmoothTestTree* tree;
  CHOOSER::IRepresentationChooser* m_reprChooser{nullptr};
  UTILS::PROPERTIES::KodiProperties m_kodiProps;
};

TEST_F(SmoothTreeTest, CalculateBaseURL)
{
  // No BaseURL tags
  OpenTestFile("ism/TearsOfSteel.ism", "http://amssamples.streaming.mediaservices.windows.net/bc57e088-27ec-44e0-ac20-a85ccbcd50da/TearsOfSteel.ism");
  EXPECT_EQ(tree->base_url_, "http://amssamples.streaming.mediaservices.windows.net/bc57e088-27ec-44e0-ac20-a85ccbcd50da/");
}

TEST_F(SmoothTreeTest, CalculateBaseURLWithNoExtension)
{
  // No BaseURL tags
  OpenTestFile("ism/TearsOfSteel.ism", "http://amssamples.streaming.mediaservices.windows.net/bc57e088-27ec-44e0-ac20-a85ccbcd50da/TearsOfSteel.ism/manifest");
  EXPECT_EQ(tree->base_url_, "http://amssamples.streaming.mediaservices.windows.net/bc57e088-27ec-44e0-ac20-a85ccbcd50da/TearsOfSteel.ism/");
}
