require 'torch'
require 'nn'

require("bench");

-- prepare batch data
mnist = require 'mnist'
trainset = mnist.traindataset()

size = 1

inputs = torch.Tensor(size, 28, 28)
for i = 1,size do
    input = trainset.data[i]:float();
    inputs[i] = input -- torch.reshape(input, 784)
end

-- construct MLP
model = nn.Sequential()

model:add(nn.Reshape(784))
model:add(nn.Linear(784, 256))
model:add(nn.Tanh())
model:add(nn.Linear(256, 10))
model:add(nn.SoftMax())

t = benchmark_median("./mlp_bench_lua.txt", 1000, function()
    y = model:forward(inputs)
end)

print(string.format("times: %f\n", t))
