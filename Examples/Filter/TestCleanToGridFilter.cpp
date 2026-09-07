#include <MyFilter/iGameCleanToGridFilter.h>
#include <iGameFileIO.h>
#include <iGameInteractor.h>
#include <iGameAttributeSet.h>
#include <iGamePointSet.h>
#include <iGameRenderWindow.h>
#include <iGameScene.h>

int main() {
    std::cout << "========== CleanToGridFilter Test ==========" << std::endl;
    
    // 1. 读取测试数据
    const std::string fileName = "./Models/Convert_Quad_Bicycle.vtk";
    
    auto obj = iGame::FileIO::ReadFile(fileName);
    if (obj == nullptr) {
        std::cout << "Read ERROR: cannot open " << fileName << std::endl;
        return 1;
    }
    
    // 2. 记录输入信息
    auto inPoints = obj->GetPoints();
    auto inCells = obj->GetCellArray();
    if (inPoints == nullptr || inCells == nullptr) {
        std::cout << "Input data has no points/cells" << std::endl;
        return 1;
    }
    
    const IGsize inPointNum = inPoints->GetNumberOfPoints();
    const IGsize inCellNum = inCells->GetNumberOfCells();
    const IGsize inAttrNum = 
        obj->GetAttributeSet() ? obj->GetAttributeSet()->GetNumberOfAttributes() : 0;
    
    std::cout << "Input  - Points: " << inPointNum << ", Cells: " << inCellNum << std::endl;
    std::cout << "Input  - Attributes: " << inAttrNum << std::endl;
    
    // 3. 执行网格清理
    auto filter = iGame::CleanToGridFilter::New();
    filter->SetInput(obj);
    filter->SetAbsoluteTolerance(0.001);
    filter->SetToleranceIsAbsolute(true);
    filter->SetMergePoints(true);
    filter->SetRemoveDegenerateCells(true);
    filter->SetCompactPointFields(true);
    
    if (!filter->Execute()) {
        std::cout << "CleanToGridFilter Execute FAILED" << std::endl;
        return 1;
    }
    
    auto out = filter->GetOutput();
    if (out == nullptr) {
        std::cout << "Output is null" << std::endl;
        return 1;
    }
    
    // 4. 验证输出结果
    auto outPoints = out->GetPoints();
    auto outCells = out->GetCellArray();
    
    const IGsize outPointNum = outPoints ? outPoints->GetNumberOfPoints() : 0;
    const IGsize outCellNum = outCells ? outCells->GetNumberOfCells() : 0;
    const IGsize outAttrNum = 
        out->GetAttributeSet() ? out->GetAttributeSet()->GetNumberOfAttributes() : 0;
    
    std::cout << "Output - Points: " << outPointNum << ", Cells: " << outCellNum << std::endl;
    std::cout << "Output - Attributes: " << outAttrNum << std::endl;
    
    // 计算变化
    long long pointDiff = (long long)inPointNum - (long long)outPointNum;
    long long cellDiff = (long long)inCellNum - (long long)outCellNum;
    double pointPercent = inPointNum > 0 ? (double)pointDiff / inPointNum * 100.0 : 0.0;
    double cellPercent = inCellNum > 0 ? (double)cellDiff / inCellNum * 100.0 : 0.0;
    
    std::cout << "Change - Points: -" << pointDiff << " (" << pointPercent << "%)" << std::endl;
    std::cout << "Change - Cells:  -" << cellDiff << " (" << cellPercent << "%)" << std::endl;
    
    // 5. 验证结果
    bool allPassed = true;
    
    // 预期：82,212 → 53,878（减少 34.5%）
    if (outPointNum == 53878) {
        std::cout << "PASS: Point count matches ParaView (53,878)" << std::endl;
    } else {
        std::cout << "FAIL: Point count mismatch. Expected 53,878, got " << outPointNum << std::endl;
        allPassed = false;
    }
    
    // 预期：96,668 → 69,436（减少 28.2%）
    if (outCellNum == 69436) {
        std::cout << "PASS: Cell count matches ParaView (69,436)" << std::endl;
    } else {
        std::cout << "FAIL: Cell count mismatch. Expected 69,436, got " << outCellNum << std::endl;
        allPassed = false;
    }
    
    // 属性数量验证
    if (outAttrNum == inAttrNum) {
        std::cout << "PASS: Attribute count preserved" << std::endl;
    } else {
        std::cout << "FAIL: Attribute count changed from " << inAttrNum << " to " << outAttrNum << std::endl;
        allPassed = false;
    }
    
    // 6. 最终结果
    if (allPassed) {
        std::cout << "\n========== ALL TESTS PASSED ==========" << std::endl;
    } else {
        std::cout << "\n========== SOME TESTS FAILED ==========" << std::endl;
        return 1;
    }
    
    // 7. 显示结果
    auto drawObj = iGame::DynamicCast<iGame::DrawObject>(out);
    if (drawObj) {
        drawObj->SetViewStyle(IG_SURFACE);
        drawObj->ConvertToDrawableData();
    }
    
    auto scene = iGame::Scene::New();
    scene->AddModel(out);
    scene->ResetCameraView();
    
    iGame::RenderWindow::Pointer window = iGame::RenderWindow::New();
    window->SetSize(1280, 720);
    window->SetScene(scene);
    
    auto interactor = iGame::Interactor::New();
    interactor->Initialize(scene);
    interactor->CreateDefaultStyle();
    window->SetInteractor(interactor);
    
    window->Show();
    
    std::cout << "\nTest completed successfully!" << std::endl;
    return 0;
}