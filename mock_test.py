import ROOT
import numpy as np

# chain = ROOT.TChain("hit_tree")
# chain.Add("/Users/matteoceschia/haystack1_10000.root")
# chain.Add("/Users/matteoceschia/haystack2_10000.root")
# chain.Add("/Users/matteoceschia/haystack3_10000.root")
#
# chain.Merge("/Users/matteoceschia/Yorck/SN-IEgenerator/ToyData/chain_30000.root")

check train test split sizes
f_true = ROOT.TFile.Open("/Users/matteoceschia/Yorck/SN-IEgenerator/ToyData/chain_60000.root")
f_cat = ROOT.TFile.Open("/Users/matteoceschia/CAT/sensitivity_CAT_chain.root")
f_imseg = ROOT.TFile.Open("/Users/matteoceschia/Yorck/ImageSegmentationClustering/ClusData/sensitivity_chain_imseg.root")

t_true = f_true.hit_tree
t_cat = f_cat.Sensitivity
t_imseg = f_imseg.Sensitivity

N = t_true.GetEntries()

print(N)
store = []
store_labels = []
y_true = []
# %%
for i in range(N):
#    if i == 998 or i ==999:
#    print("Ev N ", i)
    chain.GetEntry(i)
    #print(tree.grid_side)
    #print(tree.grid_layer)
    #print(tree.grid_column)
    side = np.array(chain.grid_side)
    layer = np.array(chain.grid_layer)
    column = np.array(chain.grid_column)
    z = np.array(chain.wirez)
    true_labels = np.array(chain.true_labels)
    true_clus = np.array(t_true.true_nclus)[0]
    #print((z+1700)/3400)
    left = np.zeros((113,9))
    right = np.zeros((113,9))
    left_labels = np.zeros((113, 9))
    right_labels = np.zeros((113 ,9))

    for j, c in enumerate(side):
        if N>40000: print(true_labels[j])
        if c:
            left[column[j], layer[j]]+=1#*(z[j]+1700)/3400
            left_labels[column[j], layer[j]]+= true_labels[j]
            # left_labels[column[j], layer[j], true_labels[j]-1]+= 1
        else:
            right[column[j], layer[j]]+=1#*(z[j]+1700)/3400
            # right_labels[column[j], layer[j], true_labels[j]-1]+= 1
            right_labels[column[j], layer[j]]+= true_labels[j]

    r_f = np.hstack((np.fliplr(left), right))
    r_f_labels = np.hstack((np.fliplr(left_labels), right_labels))
    #add padding
    t_pad = np.zeros((2, 18))
    b_pad = np.zeros((1, 18))
    padded = np.vstack((t_pad, r_f, b_pad))
    # t_pad = np.zeros((2, 18, 3))
    # b_pad = np.zeros((1, 18, 3))
    padded_labels = np.vstack((t_pad, r_f_labels, b_pad))

    v_pad = np.zeros((116,1))
    padded = np.hstack((v_pad, padded, v_pad))
    # v_pad = np.zeros((116,1, 3))
    padded_labels = np.hstack((v_pad, padded_labels, v_pad))

    store.append(np.flip(padded).reshape(padded.size)) #*255
    store_labels.append(np.flip(padded_labels).reshape(padded_labels.size))
    y_true.append(true_clus)

raw_data = np.asarray(store)
dataset = np.asarray(store).reshape(N, 116, 20, 1)
# dateset = StandardScaler().fit_transform(dataset)
dataset_labels = np.asarray(store_labels).reshape(N, 116, 20, 1)
# dataset_labels = StandardScaler().fit_transform(dataset_labels)

from sklearn.model_selection import train_test_split

X_train, X_test, y_train, y_test = train_test_split(dataset, y_true, test_size=0.1, random_state=42)
X_train, X_val, y_train, y_val = train_test_split(X_train, y_train, test_size=0.10, random_state=42)

# plt.imshow(tf.keras.preprocessing.image.array_to_img(y_train[0]))
# plt.show()

print(X_train.shape, y_train.shape)
print(X_val.shape, y_val.shape)
print(X_test.shape, y_test.shape)
