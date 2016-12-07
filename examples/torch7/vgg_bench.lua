require 'torch'
require 'nn'
require 'loadcaffe'
require 'image'

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

-- load label
function load_synset(synset)
  local file  = io.open(synset)
  local label = {}

  for line in file:lines() do
    table.insert(label, string.sub(line, 11))
  end
  return label
end

-- image preprocessing
function preprocess(data, mean)

  -- rescaling image data from 0.0 ~ 1.0 to 0.0 ~ 255.0
  local im = data * 255
  im = image.scale(im, 224, 224, 'bilinear')

  -- re order color channel  RGB -> BGR
  im = im:index(1, torch.LongTensor{3,2,1})

  return im - mean
end

-- show top 5 result
function show_top5(y, labels)
  p,classes = y:view(-1):float():sort(true)
  for i=1,5 do
    print(string.format("%d: %.6f %s", i, p[i], labels[classes[i]]))
  end
  print('\n')
end

-- load mean file
function load_mean_file(mean_file)
  f = torch.DiskFile(mean_file, "r")

  f:binary()
  f:littleEndianEncoding()

  mean = torch.Tensor(3,256,256)

  for c = 1, 3 do
    for j = 1, 256 do
      for i = 1, 256 do
        local values = f:readFloat(1)
        mean[c][i][j] = values[1]
      end
    end
  end

  f:close()

  return mean
end


-- init
torch.setnumthreads(4)

-- load synset word label for 1000 category
labels = load_synset('../../data/ilsvrc/synset_words.txt')

-- construct CNN
model = loadcaffe.load('../../data/vgg_16/VGG_ILSVRC_16_layers_deploy.prototxt', '../../data/vgg_16/VGG_ILSVRC_16_layers.caffemodel', 'nn')

-- strip Dropout layer
-- newModel = nn.Sequential()
-- for i=1,#model.modules do
--   typename = torch.typename(model.modules[i])
--   if typename ~= 'nn.Dropout' then
--      newModel:add(model.modules[i])
--   end
--end

-- disable dropout layer function
model:evaluate()

-- prepare data
mean    = torch.load('../../data/ilsvrc/ilsvrc_2012_mean.t7').img_mean:transpose(3,1)
-- mean    = load_mean_file('../../data/ilsvrc/ilsvrc12_mean.blob')
mean    = image.scale(mean, 224, 224, 'bilinear')

-- sample01.png
img01   = image.load('../../data/images/sample01.png')
input01 = preprocess(img01, mean)

-- exec prediction
-- t = benchmark_median("./bench_vgg_16_lua.txt", 10, function()
--    y = newModel:forward(input01)
-- end)

-- create header
local header = ""
for i=1,#model.modules do
   typename = torch.typename(model.modules[i])
   header = header .. string.format("%s\t", typename)
end
header = header .. "total\n"

f = io.open("dump.tsv", "w")
io.output(f)

io.write(header)

y = model:forward(input01)

io.output(io.stdout)
f:close()

show_top5(y, labels)

-- print(string.format("times: %f\n", t))

-- sample02.png
-- img02   = image.load('../../data/images/sample02.png')
-- input02 = preprocess(img02, mean)

-- y = newModel:forward(input02)
-- show_top5(y, labels)
