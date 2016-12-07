
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
