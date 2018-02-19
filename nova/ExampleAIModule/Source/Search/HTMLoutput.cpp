#include "HTMLoutput.h"

HTMLoutput::HTMLoutput(MCTSCD::gameNode_t* node, EvaluationFunction* ef)
	:_ef(ef)
{
	rootName << node;
	// create folder for all child nodes html files
	std::string childrenFolder = "bwapi-data\\logs\\" + rootName.str();
	CreateDirectory(childrenFolder.c_str(), NULL);

	printMCTSCDnode(node);
}

void HTMLoutput::createFile(std::string fileName, std::string rootPath)
{
	std::string htmlPath = "bwapi-data\\logs\\" + fileName + ".html";
	htmlFile.open(htmlPath);
	htmlFile << "<!DOCTYPE html>" << '\n';
	htmlFile << "<html>" << '\n';
	htmlFile << "<head>" << '\n';
	htmlFile << "  <meta charset=\"utf-8\">" << '\n';
	htmlFile << "  <title>MCTSCD trace log</title>" << '\n';
	htmlFile << "  <link rel=\"stylesheet\" type=\"text/css\" href=\"" << rootPath << "jquery.dataTables.css\">" << '\n';
	htmlFile << "  <script type=\"text/javascript\" language=\"javascript\" src=\"" << rootPath << "jquery.js\"></script>" << '\n';
	htmlFile << "  <script type=\"text/javascript\" language=\"javascript\" src=\"" << rootPath << "jquery.dataTables.min.js\"></script>" << '\n';
	htmlFile << "  <script type=\"text/javascript\" language=\"javascript\" class=\"init\">" << '\n';
	htmlFile << "    $(document).ready(function() {" << '\n';
	htmlFile << "      $('table.display').dataTable({" << '\n';
	htmlFile << "        \"paging\": false," << '\n';
	htmlFile << "        \"searching\": false," << '\n';
	htmlFile << "        \"info\": false" << '\n';
	htmlFile << "      });" << '\n';
	htmlFile << "    });" << '\n';
	htmlFile << "  </script>" << '\n';
	htmlFile << "</head>" << '\n';
	htmlFile << "<body>" << '\n';
}

void HTMLoutput::printMCTSCDnode(MCTSCD::gameNode_t* node)
{
	std::stringstream fileName;
	std::string rootPath("../");
	if (node->parent != nullptr) {
		fileName << rootName.str() << "\\" << node;
	} else {
		fileName << node;
		rootPath = "";
	}
	createFile(fileName.str(), rootPath);

	// print node information
	htmlFile << "<table class=\"list\">" << '\n';
	htmlFile << "  <tr>" << '\n';
	htmlFile << "    <th>Visits</th>" << '\n';
	htmlFile << "    <th>Evaluation</th>" << '\n';
	htmlFile << "    <th>Reward (Accumulative eval/visits)</th>" << '\n';
	htmlFile << "    <th>Depth</th>" << '\n';
	htmlFile << "    <th>Player</td>" << '\n';
	htmlFile << "    <th>Parent</th>" << '\n';
	htmlFile << "  </tr>" << '\n';
	htmlFile << "  <tr>" << '\n';
	htmlFile << "    <td>" << node->totalVisits << "</td>" << '\n';
	htmlFile << "    <td>" << _ef->evaluate(node->gs) << "</td>" << '\n';
	htmlFile << "    <td>" << node->totalEvaluation / node->totalVisits << "</td>" << '\n';
	htmlFile << "    <td>" << node->depth << "</td>" << '\n';
	htmlFile << "    <td>" << node->player << "</td>" << '\n';
	if (node->parent) {
		if (node->parent->parent) {
			htmlFile << "    <td><a href=\"" << node->parent << ".html\">" << node->parent << "</a></td>" << '\n';
		} else { // the parent is the root
			htmlFile << "    <td><a href=\"../" << node->parent << ".html\">" << node->parent << "</a></td>" << '\n';
		}
	} else htmlFile << "    <td>ROOT</td>" << '\n';
	htmlFile << "  </tr>" << '\n';
	htmlFile << "</table>" << '\n';

	htmlFile << "<table>" << '\n';
	htmlFile << "  <tr>" << '\n';
	htmlFile << "    <td valign=\"top\"><img src=\"" << rootPath << "map.png\"></td>" << '\n';
	htmlFile << "    <td valign=\"top\"><pre>" << '\n';
	
	htmlFile << node->gs.toString() << '\n';
	if (!node->gs.gameover() && &node->gs != node->moveGenerator._gs) {
		DEBUG("Game states are different!!");
		DEBUG(node->gs.toString());
		DEBUG(node->moveGenerator._gs->toString());
	}
	htmlFile << node->moveGenerator.toString() << '\n';

	htmlFile << "    </pre></td>" << '\n';
	htmlFile << "  </tr>" << '\n';
	htmlFile << "</table>" << '\n';

	if (node->gs.gameover())
		htmlFile << "This is a LEAF node (gameover)</br>" << '\n';
	else
		htmlFile << "Explored " << node->children.size() << " of " << node->moveGenerator._size << " children </br>" << '\n';

	// print children list
	if (!node->children.empty()) {
		htmlFile << "<table class=\"display\" cellspacing=\"0\">" << '\n';
		htmlFile << "  <thead>" << '\n';
		htmlFile << "    <tr>" << '\n';
		htmlFile << "      <th>Child</th>" << '\n';
		htmlFile << "      <th>Visits</th>" << '\n';
		htmlFile << "      <th>Evaluation</th>" << '\n';
		htmlFile << "      <th>Reward</th>" << '\n';
		htmlFile << "      <th>Player</th>" << '\n';
		htmlFile << "      <th>Action</th>" << '\n';
		htmlFile << "    </tr>" << '\n';
		htmlFile << "  </thead>" << '\n';
		htmlFile << "  <tbody>" << '\n';

		for (unsigned int i = 0; i < node->children.size(); i++) {
			MCTSCD::gameNode_t* child = node->children[i];
			htmlFile << "    <tr>" << '\n';
			if (node->parent == nullptr) { // root
				htmlFile << "      <td><a href=\"" << node << "/" << child << ".html\">" << child << "</a></td>" << '\n';
			} else {
				htmlFile << "      <td><a href=\"" << child << ".html\">" << child << "</a></td>" << '\n';
			}
			htmlFile << "      <td>" << child->totalVisits << "</td>" << '\n';
			htmlFile << "      <td>" << _ef->evaluate(child->gs) << "</td>" << '\n';
			htmlFile << "      <td>" << child->totalEvaluation / child->totalVisits << "</td>" << '\n';
			htmlFile << "      <td>" << child->player << "</td>" << '\n';
			htmlFile << "      <td>" << node->moveGenerator.toString(node->actions[i]) << "</td>" << '\n';
			htmlFile << "    </tr>" << '\n';
		}

		htmlFile << "  </tbody>" << '\n';
		htmlFile << "</table>" << '\n';
	}

	// print simulations
	if (!node->simulations.empty()) {
		htmlFile << "<h2>" << node->simulations.size() << " Simulations</h2>" << '\n';
		htmlFile << "<table class=\"display\" cellspacing=\"0\">" << '\n';
		htmlFile << "  <thead>" << '\n';
		htmlFile << "    <tr>" << '\n';
		htmlFile << "      <th>Evaluation</th>" << '\n';
		htmlFile << "      <th>Game State</th>" << '\n';
		htmlFile << "    </tr>" << '\n';
		htmlFile << "  </thead>" << '\n';
		htmlFile << "  <tbody>" << '\n';
		for (auto& simulation : node->simulations) {
			htmlFile << "    <tr>" << '\n';
			htmlFile << "      <td>" << _ef->evaluate(simulation) << "</td>" << '\n';
			htmlFile << "      <td><pre>" << simulation.toString() << "</pre>" << '\n';
			htmlFile << "    </tr>" << '\n';
		}
		htmlFile << "  </tbody>" << '\n';
		htmlFile << "</table>" << '\n';
	}

	closeFile();

	// print children nodes
	for (const auto& childNode : node->children) {
		HTMLoutput::printMCTSCDnode(childNode);
	}
}

void HTMLoutput::closeFile()
{
	htmlFile << "</body>" << '\n';
	htmlFile << "</html>" << std::endl;
	htmlFile.close();
}