require 'torch'
require 'nn'

function benchmark_median(path, samples, func)
  local result = {}
  for n=1, samples do
    t1 = os.clock()
    func()
    t = os.clock() - t1

    table.insert(result, n, t)
  end

  local median;
  table.sort(result)

  -- output results
  f = io.open(path, "w")

  for n=1, samples do
    f:write(string.format("%f", result[n]))
    f:write('\n')
  end

  f:close()

  -- calc median
  if samples % 2 == 0 then
    median = (result[samples / 2 - 1] + result[samples / 2]) / 2
  else
    median = result[samples / 2]
  end

  return median
end

-- prepare batch data
mnist = require 'mnist'
trainset = mnist.traindataset()

size = 1

inputs = torch.Tensor(size, 1, 28, 28)
for i = 1,size do
    input = trainset.data[i]:float()
    inputs[i] = torch.reshape(input, 1, 28, 28)
end

-- construct CNN
model = nn.Sequential()

-- conv1 (1x28x28 -> 32x24x24) -> max pooling(32x24x24 -> 32x8x8)
model:add(nn.SpatialConvolutionMM(1, 32, 5, 5))
model:add(nn.Tanh())
model:add(nn.SpatialMaxPooling(3, 3, 3, 3))

-- conv2 (32x8x8 -> 64x6x6) -> max pooling(64x6x6 -> 64x3x3)
model:add(nn.SpatialConvolutionMM(32, 64, 3, 3))
model:add(nn.Tanh())
model:add(nn.SpatialMaxPooling(2, 2, 2, 2))

-- MLP
model:add(nn.Reshape(64*3*3))
model:add(nn.Linear(64*3*3, 256))
model:add(nn.Tanh())
model:add(nn.Linear(256, 10))
model:add(nn.SoftMax())

t = benchmark_median("./cnn_bench_lua.txt", 1000, function()
    y = model:forward(inputs)
end)

print(string.format("times: %f\n", t))
