import numpy as np
from graphviz import Digraph


train_set = np.load('dt/train_set.npy')
train_labels = np.load('dt/train_labels.npy')
test_set = np.load('dt/test_set.npy')
test_labels = np.load('dt/test_labels.npy')

#first 10 df and 90 confidence level chi squared table
chiSquaredTable = [2.706,4.605,6.251,7.779,9.236,10.645,12.017,13.362,14.684,15.987]
def entropy(bucket):
    """
    Calculates the entropy.
    :param bucket: A list of size num_classes. bucket[i] is the number of
    examples that belong to class i.
    :return: A float. Calculated entropy.
    """
    totalVal = 0
    for noExamples in bucket:
        totalVal += noExamples
    entropy = 0
    for noExamples in bucket:
        if(noExamples != 0):
            entropy -= (noExamples / totalVal) * np.log2(noExamples / totalVal)
    return entropy

def info_gain(parent_bucket, left_bucket, right_bucket):
    """
    Calculates the information gain. A bucket is a list of size num_classes.
    bucket[i] is the number of examples that belong to class i.
    :param parent_bucket: Bucket belonging to the parent node. It contains the
    number of examples that belong to each class before the split.
    :param left_bucket: Bucket belonging to the left child after the split.
    :param right_bucket: Bucket belonging to the right child after the split.
    :return: A float. Calculated information gain.
    """
    totalValParent = 0
    totalValLeft = 0
    totalValRight = 0
    for valParent in parent_bucket:
        totalValParent += valParent
    for valLeft in left_bucket:
        totalValLeft += valLeft
    for valRight in right_bucket:
        totalValRight += valRight
    infoGain = entropy(parent_bucket) - (totalValLeft / totalValParent) * entropy(left_bucket) - (totalValRight / totalValParent) * entropy(right_bucket)
    return infoGain
def gini(bucket):
    """
    Calculates the gini index.
    :param bucket: A list of size num_classes. bucket[i] is the number of
    examples that belong to class i.
    :return: A float. Calculated gini index.
    """
    totalVal = 0
    for noExamples in bucket:
        totalVal += noExamples
    giniIndex = 1
    for noExamples in bucket:
        if (noExamples != 0):
            giniIndex -= (noExamples / totalVal) ** 2
    return giniIndex

def avg_gini_index(left_bucket, right_bucket):
    """
    Calculates the average gini index. A bucket is a list of size num_classes.
    bucket[i] is the number of examples that belong to class i.
    :param left_bucket: Bucket belonging to the left child after the split.
    :param right_bucket: Bucket belonging to the right child after the split.
    :return: A float. Calculated average gini index.
    """
    totalValLeft = 0
    totalValRight = 0
    for valLeft in left_bucket:
        totalValLeft += valLeft
    for valRight in right_bucket:
        totalValRight += valRight
    totalValParent = totalValLeft + totalValRight
    avgGini = (totalValLeft / totalValParent) * gini(left_bucket) + (totalValRight / totalValParent) * gini(right_bucket)
    return avgGini
def calculate_split_values(data, labels, num_classes, attr_index, heuristic_name):
    """
    For every possible values to split the data for the attribute indexed by
    attribute_index, it divides the data into buckets and calculates the values
    returned by the heuristic function named heuristic_name. The split values
    should be the average of the closest 2 values. For example, if the data has
    2.1 and 2.2 in it consecutively for the values of attribute index by attr_index,
    then one of the split values should be 2.15.
    :param data: An (N, M) shaped numpy array. N is the number of examples in the
    current node. M is the dimensionality of the data. It contains the values for
    every attribute for every example.
    :param labels: An (N, ) shaped numpy array. It contains the class values in
    it. For every value, 0 <= value < num_classes.
    :param num_classes: An integer. The number of classes in the dataset.
    :param attr_index: An integer. The index of the attribute that is going to
    be used for the splitting operation. This integer indexs the second dimension
    of the data numpy array.
    :param heuristic_name: The name of the heuristic function. It should either be
    'info_gain' of 'avg_gini_index' for this homework.
    :return: An (L, 2) shaped numpy array. L is the number of split values. The
    first column is the split values and the second column contains the calculated
    heuristic values for their splits.
    """
    numbersForSplit = []
    for dataIndex in data:
        numbersForSplit.append(dataIndex[attr_index])
    numbersForSplit.sort()
    splitValues = []
    for i in range(0,len(numbersForSplit)):
        if(i != len(numbersForSplit) - 1):
            splitValues.append((numbersForSplit[i] + numbersForSplit[i+1]) / 2)

    labels = labels.tolist()
    parentBucket = []
    for i in range(0,num_classes):
        parentBucket.append(labels.count(i))
    heuricticValues = []
    for splitValue in splitValues:
        leftLabels = []
        rightLabels = []
        for index in range(0,len(data)):
            if(data[index][attr_index] < splitValue):
                leftLabels.append(labels[index])
            else:
                rightLabels.append(labels[index])
        leftBucket = []
        rightBucket = []
        for i in range(0, num_classes):
            leftBucket.append(leftLabels.count(i))
            rightBucket.append(rightLabels.count(i))
        if(heuristic_name == 'info_gain'):
            heuricticValues.append(info_gain(parentBucket,leftBucket,rightBucket))
        elif(heuristic_name == 'avg_gini_index'):
            heuricticValues.append(avg_gini_index(leftBucket,rightBucket))
    result = np.empty(0)
    for i in range(0,len(splitValues)):
        temp = []
        temp.append(splitValues[i])
        temp.append(heuricticValues[i])
        nptemp = np.array(temp)
        if(i == 0):
            result = np.concatenate((result,nptemp),axis=0)
        else:
            result = np.vstack((result,nptemp))
    return result




def chi_squared_test(left_bucket, right_bucket):
    """
    Calculates chi squared value and degree of freedom between the selected attribute
    and the class attribute. A bucket is a list of size num_classes. bucket[i] is the
    number of examples that belong to class i.
    :param left_bucket: Bucket belonging to the left child after the split.
    :param right_bucket: Bucket belonging to the right child after the split.
    :return: A float and and integer. Chi squared value and degree of freedom.
    """
    rowsTotal = []
    columnsTotal = []
    grandTotal = 0
    row = 0
    for i in range(0,len(left_bucket)):
        column = 0
        grandTotal += left_bucket[i]
        row += left_bucket[i]
        column += left_bucket[i] + right_bucket[i]
        columnsTotal.append(column)
    rowsTotal.append(row)
    row = 0
    for k in range(0,len(right_bucket)):
        grandTotal += right_bucket[k]
        row += right_bucket[k]
    rowsTotal.append(row)
    chi_squared = 0
    for x in range(0,len(rowsTotal)):
        for y in range(0,len(columnsTotal)):
            if (x == 0): #left bucket
                expValue = (rowsTotal[x] * columnsTotal[y]) / grandTotal
                sqr_difference = (left_bucket[y] - expValue) ** 2
                if (expValue != 0):
                    chi_squared += sqr_difference / expValue
            elif (x == 1): #right bucket
                expValue = (rowsTotal[x] * columnsTotal[y]) / grandTotal
                sqr_difference = (right_bucket[y] - expValue) ** 2
                if (expValue != 0):
                    chi_squared += sqr_difference / expValue
    df = len(columnsTotal) - 1
    for i in range (0,len(left_bucket)):
        if(left_bucket[i] == 0 and right_bucket[i] == 0):
            df -= 1
    return chi_squared,df





class Tree:
    def __init__(self):
        self.left = None
        self.right = None
        self.splitVal = None
        self.labels = None
        self.id = None
        self.splitIndex = None


idNo = 0
def getBucket(labels,num_classes):
    bucket = []
    for _ in range(0,num_classes):
        bucket.append(0)
    for i in labels:
        bucket[i] += 1
    return bucket

#detects if a bucket has labels more than one class
def labelDistribution(bucket):
    zeroCount = 0
    for i in range(0,len(bucket)):
        if(bucket[i] == 0):
            zeroCount += 1
    if(len(bucket) - zeroCount == 1):
        return False
    elif(len(bucket) - zeroCount > 1):
        return True
def formTree(tr,data,labels,num_classes,heuristic_name,prepruning):
    global idNo
    if(getSplitVal(data,labels,num_classes,heuristic_name) == 0):
        tr.labels = getBucket(labels,num_classes)
        tr.id = idNo
        idNo += 1
    else:
        if(growTree(data,labels,num_classes,heuristic_name,prepruning) == 0):
            tr.labels = getBucket(labels, num_classes)
            tr.id = idNo
            idNo += 1
        else:
            leftData,leftLabel,rightData,rightLabel = growTree(data,labels,num_classes,heuristic_name,prepruning)
            splitVal,splitIndex = getSplitVal(data,labels,num_classes,heuristic_name)
            tr.splitVal = splitVal
            tr.labels = getBucket(labels,num_classes)
            tr.id = idNo
            tr.splitIndex = splitIndex
            idNo += 1
            leftTree = Tree()
            rightTree = Tree()
            tr.left = leftTree
            tr.right = rightTree
            formTree(leftTree,leftData,leftLabel,num_classes,heuristic_name,prepruning)
            formTree(rightTree,rightData,rightLabel,num_classes,heuristic_name,prepruning)




def getSplitVal(data,labels,num_classes,heuristic_name):
    if (len(data.shape) != 1 and len(data) != 0):

        splitValue = 0
        attrIndex = 0
        if (heuristic_name == 'info_gain'):
            bucket = getBucket(labels, num_classes)
            e = entropy(bucket)
            if(e != 0.0):
                max = 0
                val = 0
                for k in range(0,len(train_set[0])):
                    splits = calculate_split_values(data, labels, num_classes, k, heuristic_name)
                    if (len(splits.shape) != 1):
                        for i in range(0, len(splits)):
                            if (splits[i][1] > max):
                                max = splits[i][1]
                                splitValue = splits[i][0]
                                attrIndex = k
                        if (max == 0.0):
                            return 0
                    else:
                        if(val < splits[1]):
                            splitValue = splits[0]
                            attrIndex = k
                            val = splits[1]
                return splitValue,attrIndex
            else:
                return 0

        elif (heuristic_name == 'avg_gini_index'):
            bucket = getBucket(labels,num_classes)
            totalVal = 0
            for noExamples in bucket:
                totalVal += noExamples
            giniIndex = 1
            for noExamples in bucket:
                if (noExamples != 0):
                    giniIndex -= (noExamples / totalVal) ** 2
            min = 999999
            val = 999
            if(giniIndex != 0.0):
                for k in range(0, len(train_set[0])):
                    splits = calculate_split_values(data, labels, num_classes, k, heuristic_name)
                    if (len(splits.shape) != 1):
                        for i in range(0, len(splits)):
                            if (splits[i][1] < min and splits[i][1] != 0.0):
                                min = splits[i][1]
                                splitValue = splits[i][0]
                                attrIndex = k

                    else:
                        if(val > splits[1]):
                            splitValue = splits[0]
                            attrIndex = k
                            val = splits[1]
                return splitValue,attrIndex
            else:
                return 0


    else:
        return 0
def growTree(data,labels, num_classes, heuristic_name,prepruning):

    splitValue,attrIndex = getSplitVal(data,labels,num_classes,heuristic_name)
    #separating data into left and right
    leftBranch = []
    rightBranch = []
    for i in range (0,len(data)):
        if(data[i][attrIndex] < splitValue):
            tempTuple = (data[i],labels[i])
            leftBranch.append(tempTuple)
        else:
            tempTuple = (data[i],labels[i])
            rightBranch.append(tempTuple)
    leftData = np.empty((0))
    leftLabels = []
    rightData = np.empty((0))
    rightLabels = []
    for i in range(0,len(leftBranch)):
        leftLabels.append(leftBranch[i][1])
        if(i == 0):
            leftData = np.concatenate((leftData,leftBranch[i][0]),axis=0)
        else:
            leftData = np.vstack((leftData,leftBranch[i][0]))
    for i in range(0,len(rightBranch)):
        rightLabels.append(rightBranch[i][1])
        if(i == 0):
            rightData = np.concatenate((rightData,rightBranch[i][0]),axis=0)
        else:
            rightData = np.vstack((rightData,rightBranch[i][0]))


    #pre pruning
    if(prepruning == 0):
        leftLabels = np.array(leftLabels)
        rightLabels = np.array(rightLabels)
        return leftData, leftLabels, rightData, rightLabels
    else:
        leftBucket = getBucket(leftLabels,num_classes)
        rightBucket = getBucket(rightLabels,num_classes)
        chi_sqr,df = chi_squared_test(leftBucket,rightBucket)

        if(chi_sqr < chiSquaredTable[df - 1]):
            return 0
        else:
            leftLabels = np.array(leftLabels)
            rightLabels = np.array(rightLabels)

            return leftData,leftLabels,rightData,rightLabels

def applyPostPruning(tree):
    if(tree.left == None and tree.right == None):
        return
    else:
        l = labelDistribution(tree.left.labels)
        r = labelDistribution(tree.right.labels)
        if(l == False and r == True):
            if(tree.right.left == None and tree.right.right == None):
                tree.right = None
            else:
                applyPostPruning(tree.right)
        elif(l == True and r == False):
            if (tree.left.left == None and tree.left.right == None):
                tree.left = None
            else:
                applyPostPruning(tree.left)
        elif(l == True and r == True):

            totalExample = 0
            lExample = 0
            rExample = 0
            for i in range(0,len(tree.labels)):
                totalExample += tree.labels[i]
                lExample += tree.left.labels[i]
                rExample += tree.right.labels[i]
            if(lExample >= ((totalExample * 85) / 100)):
                tree.right = None
                applyPostPruning(tree.left)
            elif(rExample >= ((totalExample * 85) / 100)):
                tree.left = None
                applyPostPruning(tree.right)
            else:
                applyPostPruning(tree.left)
                applyPostPruning(tree.right)

def graphReprTree(dot,tree,graphIndex):

    if(tree.left == None and tree.right == None):
        return
    else:
        if(tree.left == None and tree.right != None):
            a = str(tree.id)
            c = str(tree.right.id)
            dot.node(a, label='x[' + str(tree.splitIndex) + ']' + ' < ' + str(tree.splitVal) + '\n' + '(' + str(tree.labels) + ')')
            dot.node(c, str(tree.right.labels))
            dot.edge(a, c, label='>=')
            graphReprTree(dot, tree.right, graphIndex)
        elif(tree.left != None and tree.right == None):
            a = str(tree.id)
            b = str(tree.left.id)
            dot.node(a, label='x[' + str(tree.splitIndex) + ']' + ' < ' + str(tree.splitVal) + '\n' + '(' + str(tree.labels) + ')')
            dot.node(b, str(tree.left.labels))
            dot.edge(a, b, label='<')
            graphReprTree(dot,tree.left,graphIndex)
        else:
            a = str(tree.id)
            b = str(tree.left.id)
            c = str(tree.right.id)
            dot.node(a,label= 'x[' + str(tree.splitIndex) + ']' + ' < ' + str(tree.splitVal) + '\n' + '(' + str(tree.labels) + ')')
            dot.node(b,str(tree.left.labels))
            dot.node(c,str(tree.right.labels))
            dot.edge(a,b,label='<' )
            dot.edge(a,c,label='>=')
            graphReprTree(dot,tree.left,graphIndex)
            graphReprTree(dot,tree.right,graphIndex)


predictedLabels = []
def getTestAccuracy(example,tree):
    if(tree.left == None and tree.right == None):
        maxVal = max(tree.labels)
        maxIndex = tree.labels.index(maxVal)
        predictedLabels.append(maxIndex)
    else:
        splitIndex = tree.splitIndex
        if(example[splitIndex] < tree.splitVal):
            getTestAccuracy(example,tree.left)
        else:
            getTestAccuracy(example,tree.right)



## TESTING


dt = Tree()

#if you don't want to prepruning,you can choose last parameter as 0, if you want choose as 1
formTree(dt,train_set,train_labels,3,'avg_gini_index',1)

accuracy = 0
for i in test_set:
    getTestAccuracy(i,dt)
for i in range(0,len(test_labels)):
    if(test_labels[i] == predictedLabels[i]):
        accuracy += 1
print((accuracy / len(test_labels)) * 100)

dot = Digraph()
graphReprTree(dot,dt,0)
dot.render(filename="test",view=True)



