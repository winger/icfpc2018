#include "grounder.h"

namespace {
int TESTS = 186;
const std::vector< std::vector<int> > DIRS_2D {
  {-1, 0, 0},
  {1, 0, 0},
  {0, 0, -1},
  {0, 0, 1}
};

const std::vector< std::vector<int> > DIRS_3D {
  {-1, 0, 0},
  {1, 0, 0},
  {0, -1, 0},
  {0, 1, 0},
  {0, 0, -1},
  {0, 0, 1}
};

}


bool Grounder::IsDeltaGrounded(
    Matrix const& model, std::vector<uint32_t> const& indicies) {
  std::unordered_set<uint32_t> was;
  std::unordered_set<uint32_t> delta;
  std::vector<uint32_t> queue;
  for (auto v: indicies) {
    delta.insert(v);
    auto coords = model.Reindex(v);
    if (coords[1] == 0) {
      was.insert(v);
      queue.push_back(v);
      continue;
    }
    for (auto const& dir: DIRS_3D) {
      auto nc = coords;
      for (int i = 0; i < dir.size(); ++i) {
        nc[i] += dir[i];
      }
      if (model.IsInside(nc[0], nc[1], nc[2]) &&
          model.Get(nc[0], nc[1], nc[2])) {
        was.insert(v);
        queue.push_back(v);
        break;
      }
    }
  }

  if (queue.size() == delta.size()) {
    return true;
  }

  for (int begin = 0; begin < queue.size(); ++begin) {
    auto coords = model.Reindex(queue[begin]);
    for (auto const& dir: DIRS_3D) {
      auto nc = coords;
      for (int i = 0; i < dir.size(); ++i) {
        nc[i] += dir[i];
      }
      auto index = model.Index(nc[0], nc[1], nc[2]);
      if (model.IsInside(nc[0], nc[1], nc[2]) &&
          delta.find(index) != delta.end()) {
        was.insert(index);
        queue.push_back(index);
      }
    }
  }
  return queue.size() == delta.size();
}

bool Grounder::IsGrounded(Matrix const& model) {
  auto numOfFull = model.FullNum();
  auto size = model.GetR();
  auto volume = model.GetVolume();

  std::unordered_set<uint32_t> was;
  std::vector<uint32_t> queue;
  for (int x = 0; x < size; ++x) {
    for (int z = 0; z < size; ++z) {
      if (model.Get(x, 0, z) == 1) {
        auto index = model.Index(x, 0, z);
        was.insert(index);
        queue.push_back(index);
      }
    }
  }
  for (int begin = 0; begin < queue.size(); ++begin) {
    auto coords = model.Reindex(queue[begin]);
    for (auto const& dir: DIRS_3D) {
      auto nc = coords;
      for (int i = 0; i < dir.size(); ++i) {
        nc[i] += dir[i];
      }
      auto index = model.Index(nc[0], nc[1], nc[2]);
      if (model.IsInside(nc[0], nc[1], nc[2]) &&
          model.Get(nc[0], nc[1], nc[2]) &&
          was.find(index) == was.end()) {
        was.insert(index);
        queue.push_back(index);
      }
    }
  }

  return queue.size() == numOfFull;
}

bool Grounder::IsByLayerGrounded(Matrix const& model) {
  auto numOfFull = model.FullNum();
  auto size = model.GetR();
  auto volume = model.GetVolume();

  std::vector<uint8_t> was(volume, 0);
  std::vector<uint32_t> queue;
  for (int x = 0; x < size; ++x) {
    for (int z = 0; z < size; ++z) {
      if (model.Get(x, 0, z) == 1) {
        auto index = model.Index(x, 0, z);
        was[index] = 1;
        queue.push_back(index);
      }
    }
  }
  int begin = 0;
  for (int y = 1; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      for (int z = 0; z < size; ++z) {
        if (model.Get(x, y, z) == 1 && was[model.Index(x, y - 1, z)]) {
          auto index = model.Index(x, y, z);
          was[index] = 1;
          queue.push_back(index);
        }
      }
    }
    for (; begin < queue.size(); ++begin) {
      auto coords = model.Reindex(queue[begin]);
      for (auto const& dir: DIRS_2D) {
        auto nc = coords;
        for (int i = 0; i < dir.size(); ++i) {
          nc[i] += dir[i];
        }
        auto index = model.Index(nc[0], nc[1], nc[2]);
        if (model.IsInside(nc[0], nc[1], nc[2]) &&
            model.Get(nc[0], nc[1], nc[2]) &&
            not was[index]) {
          was[index] = true;
          queue.push_back(index);
        }
      }
    }
  }
  return queue.size() == numOfFull;
}

bool Grounder::IsProjectionGrounded(Matrix const& model) {
  auto size = model.GetR();
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size - 1; ++y) {
      for (int z = 0; z < size; ++z) {
        if (model.Get(x, y, z) == 0 && model.Get(x, y + 1, z) == 1) {
          return false;
        }
      }
    }
  }
  return true;
}

bool Grounder::Check(unsigned model_index)
{
    string si = to_string(1000 + model_index).substr(1);
    string name = "LA" + si + "_tgt";
    Matrix model;
    model.ReadFromFile(name);
    bool result = IsProjectionGrounded(model);
    cout << name << "," << result << endl;
    return result;
}

void Grounder::CheckAll()
{
    unsigned num = 0;
    for (unsigned i = 1; i <= TESTS; ++i) {
      num += Check(i) ? 1 : 0;
    }
    cout << "Final score: " << num << " / " << TESTS << endl;
}
