/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_FEDERATED_SYNTHETIC_DATASET_SYNTHETIC_DATASET_H_
#define BRAVE_COMPONENTS_BRAVE_FEDERATED_SYNTHETIC_DATASET_SYNTHETIC_DATASET_H_

#include <string>
#include <vector>

namespace brave_federated {

class SyntheticDataset {
 public:
  // Generates the synthetic dataset of size |size| around given vector |m| of
  // size ms_size and given bias |b|.
  SyntheticDataset(std::vector<float> ms, float b, size_t size);
  SyntheticDataset(float alpha, float beta, int num_features, size_t size);
  explicit SyntheticDataset(std::vector<std::vector<float>> data_points);
  SyntheticDataset(std::vector<std::vector<float>> W,
                   std::vector<float> b,
                   int num_features,
                   size_t size);

  ~SyntheticDataset();

  size_t size();
  int get_features_count();
  std::vector<std::vector<float>> GetDataPoints();

  SyntheticDataset SeparateTestData(int num_training);
  void DumpToCSV(std::string prefix);

 private:
  float Softmax(float z);

  std::vector<std::vector<float>> data_points_;
};

}  // namespace brave_federated

#endif  // BRAVE_COMPONENTS_BRAVE_FEDERATED_SYNTHETIC_DATASET_SYNTHETIC_DATASET_H_
