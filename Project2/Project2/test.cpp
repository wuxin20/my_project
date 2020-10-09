/**
* Definition for binary tree
* struct TreeNode {
*     int val;
*     TreeNode *left;
*     TreeNode *right;
*     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
* };
*/
class Solution {
public:
	TreeNode* reConstructBinaryTree(vector<int> pre, vector<int> vin) {
		int vinlen = vin.size();
		if (vinlen == 0)
			return NULL;
		vector<int> leftpre, rightpre, leftvin, rightvin;
		TreeNode* head = new TreeNode(pre[0]);
		int root = 0;
		for (int i = 0; i<vinlen; i++){
			if (vin[i] == pre[0]){
				root = i;
				break;
			}
		}
		for (int i = 0; i<root; i++){
			leftpre.push_back(pre[i + 1]);
			leftvin.push_back(vin[i]);

		}
		for (int i = root + 1; i<vinlen; i++){
			rightpre.push_back(pre[i]);
			rightvin.push_back(vin[i]);
		}
		head->left = reConstructBinaryTree(leftpre, leftvin);
		head->right = reConstructBinaryTree(rightpre, rightvin);
		return head;

	}
};