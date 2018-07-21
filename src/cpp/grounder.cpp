#include "grounder.h"

int TESTS = 186;


bool Grounder::IsGrounded(Matrix const& model) {
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
  std::vector< std::vector<int> > dirs{
    {-1, 0, 0},
    {1, 0, 0},
    {0, 0, -1},
    {0, 0, 1}
  };
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
      for (auto const& dir: dirs) {
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

bool Grounder::Check(unsigned model_index)
{
    string si = to_string(1000 + model_index).substr(1);
    string name = "LA" + si + "_tgt";
    Matrix model;
    model.ReadFromFile(name);
    bool result = IsGrounded(model);
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
